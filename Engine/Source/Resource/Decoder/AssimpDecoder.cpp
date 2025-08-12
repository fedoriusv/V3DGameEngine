#include "AssimpDecoder.h"

#include "Renderer/Formats.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#include "Resource/Model.h"
#include "Resource/Bitmap.h"
#include "Resource/Loader//ModelFileLoader.h"
#include "Resource/Loader/ImageFileLoader.h"
#include "Resource/ResourceManager.h"
#include "Stream/FileLoader.h"

#ifdef USE_ASSIMP
#   include <assimp/Importer.hpp>
#   include <assimp/scene.h>
#   include <assimp/postprocess.h>
#   include <assimp/cimport.h>

#define LOG_LOADIMG_TIME (DEBUG || 1)

namespace v3d
{
namespace resource
{

const ModelFileLoader::VertexProperiesFlags k_defaultVertexProps =
    toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Position) |
    toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Normals) |
    toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Tangent) |
    toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Bitangent) |
    toEnumType(ModelFileLoader::VertexProperies::VertexProperies_TextCoord0) |
    0;

AssimpDecoder::AssimpDecoder(const std::vector<std::string>& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
{
}

AssimpDecoder::AssimpDecoder(std::vector<std::string>&& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
{
}

AssimpDecoder::~AssimpDecoder()
{
}

Resource* AssimpDecoder::decode(const stream::Stream* stream, const Policy* policy, u32 flags, const std::string& name) const
{
    if (stream->size() > 0)
    {
        stream->seekBeg(0);

#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif //LOG_LOADIMG_TIME

        const ModelFileLoader::ModelPolicy* modlePolicy = static_cast<const ModelFileLoader::ModelPolicy*>(policy);
        ModelFileLoader::VertexProperiesFlags vertexProps = modlePolicy->vertexProperies ? modlePolicy->vertexProperies : k_defaultVertexProps;

        u32 assimpFlags =
            aiProcess_ValidateDataStructure |
            aiProcess_ImproveCacheLocality |
            aiProcess_RemoveRedundantMaterials |
            aiProcess_FindDegenerates |
            aiProcess_FindInvalidData |
            aiProcess_LimitBoneWeights |
            aiProcess_OptimizeMeshes |
            aiProcess_Triangulate |
            aiProcess_ConvertToLeftHanded |
            aiProcess_SortByPType |
            aiProcess_GlobalScale |
            aiProcess_GenBoundingBoxes |
            0;

        if (!(flags & ModelFileLoader::SkipIndexBuffer))
        {
            assimpFlags |= aiProcess_JoinIdenticalVertices;
        }

        if (vertexProps & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Normals))
        {
            assimpFlags |= aiProcess_GenSmoothNormals;
        }

        if ((vertexProps & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Tangent)) || (vertexProps & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Bitangent)))
        {
            assimpFlags |= aiProcess_CalcTangentSpace;
        }

        if ((vertexProps & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_TextCoord0)) || (vertexProps & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_TextCoord1)) ||
            (vertexProps & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_TextCoord2)) || (vertexProps & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_TextCoord3)))
        {
            assimpFlags |= aiProcess_GenUVCoords | aiProcess_TransformUVCoords;
        }

        if (flags & ModelFileLoader::SplitLargeMeshes)
        {
            assimpFlags |= aiProcess_SplitLargeMeshes;
        }

        if (!(flags & ModelFileLoader::LocalTransform))
        {
            assimpFlags |= aiProcess_PreTransformVertices;
        }

        const u8* data = stream->map(stream->size());
        ASSERT(data, "nullptr");

        Assimp::Importer Importer;
        Importer.SetPropertyInteger(AI_CONFIG_IMPORT_TER_MAKE_UVS, 1);
        Importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
        Importer.SetPropertyInteger(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
        Importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.f);

        std::string fileExtension = stream::FileLoader::getFileExtension(name);
        const aiScene* scene = Importer.ReadFileFromMemory(data, stream->size(), assimpFlags, fileExtension.c_str());
        stream->unmap();
        if (!scene)
        {
            LOG_ERROR("AssimpDecoder::decode: Load of model [%s] is failed: %d", name.c_str(), Importer.GetErrorString());

            ASSERT(false, "nullptr");
            return nullptr;
        }


        ModelResource::ModelContentFlags contentFlags = 0;
        if (scene->HasMeshes())
        {
            contentFlags |= ModelResource::ModelContent_Meshes;
        }
        if (scene->HasMaterials() && !(flags & ModelFileLoader::SkipMaterial))
        {
            contentFlags |= ModelResource::ModelContent_Materials;
        }
        if (scene->HasLights())
        {
            contentFlags |= ModelResource::ModelContent_Lights;
        }

        u32 modelStreamSize = 0;
        stream::Stream* modelStream = stream::StreamManager::createMemoryStream();

        modelStream->write<ModelResource::ModelContentFlags>(contentFlags);
        if (contentFlags & ModelResource::ModelContent_Meshes)
        {
            modelStreamSize += AssimpDecoder::decodeMesh(scene, modelStream, flags, vertexProps);
        }

        if (contentFlags & ModelResource::ModelContent_Materials)
        {
            modelStreamSize += AssimpDecoder::decodeMaterial(scene, modelStream);
            contentFlags |= ModelResource::ModelContent_Materials;
        }

        if (contentFlags & ModelResource::ModelContent_Lights)
        {
            modelStreamSize += AssimpDecoder::decodeLight(scene, modelStream);
            contentFlags |= ModelResource::ModelContent_Lights;
        }

        ModelResource::ModelHeader header;
        ResourceHeader::fill(&header, name, modelStreamSize, 0);

        Resource* model = V3D_NEW(resource::ModelResource, memory::MemoryLabel::MemoryObject)(header);
        if (!model->load(modelStream))
        {
            LOG_ERROR("MeshAssimpDecoder::decode: the model %s loading is failed", name.c_str());

            V3D_DELETE(model, memory::MemoryLabel::MemoryObject);
            model = nullptr;
        }
        stream::StreamManager::destroyStream(modelStream);

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_INFO("MeshAssimpDecoder::decode: the model %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

        return model;
    }

    ASSERT(false, "empty");
    return nullptr;
}

u32 AssimpDecoder::decodeMesh(const aiScene* scene, stream::Stream* modelStream, ModelFileLoader::ModelLoaderFlags flags, u32 vertexPropFlags) const
{
    ASSERT(modelStream, "nullptr");
    modelStream->write<u32>(scene->mNumMeshes);

    u32 streamMeshesSize = sizeof(u32);
    for (u32 m = 0; m < scene->mNumMeshes; m++)
    {
        std::vector<renderer::VertexInputAttributeDesc::InputAttribute> inputAttributes;
        static auto buildVertexData = [&inputAttributes](const aiMesh* mesh, ModelFileLoader::VertexProperiesFlags presentFlags) -> u32
        {
            u32 vertexSize = 0;
            if (presentFlags & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Position))
            {
                ASSERT(mesh->HasPositions(), "must be resented");
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Position vec3");

                renderer::VertexInputAttributeDesc::InputAttribute attrib;
                attrib._binding = 0;
                attrib._stream = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offset = vertexSize;
                inputAttributes.push_back(attrib);

                vertexSize += sizeof(math::float3);
            }


            if (presentFlags & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Normals))
            {
                ASSERT(mesh->HasNormals(), "must be resented");
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Normals vec3");

                renderer::VertexInputAttributeDesc::InputAttribute attrib;
                attrib._binding = 0;
                attrib._stream = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offset = vertexSize;
                inputAttributes.push_back(attrib);

                vertexSize += sizeof(math::float3);
            }

            if (presentFlags & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Tangent))
            {
                ASSERT(mesh->HasTangentsAndBitangents(), "must be resented");
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Tangent vec3");

                renderer::VertexInputAttributeDesc::InputAttribute attrib;
                attrib._binding = 0;
                attrib._stream = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offset = vertexSize;
                inputAttributes.push_back(attrib);

                vertexSize += sizeof(math::float3);
            }

            if (presentFlags & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Bitangent))
            {
                ASSERT(mesh->HasTangentsAndBitangents(), "must be resented");
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Bitangent vec3");

                renderer::VertexInputAttributeDesc::InputAttribute attrib;
                attrib._binding = 0;
                attrib._stream = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offset = vertexSize;
                inputAttributes.push_back(attrib);

                vertexSize += sizeof(math::float3);
            }

            static const ModelFileLoader::VertexProperies uvFlag[k_maxTextureCoordsCount] =
            {
                ModelFileLoader::VertexProperies::VertexProperies_TextCoord0,
                ModelFileLoader::VertexProperies::VertexProperies_TextCoord1,
                ModelFileLoader::VertexProperies::VertexProperies_TextCoord2,
                ModelFileLoader::VertexProperies::VertexProperies_TextCoord3
            };

            for (u32 uv = 0; uv < k_maxTextureCoordsCount; uv++)
            {
                if (presentFlags & toEnumType(uvFlag[uv]))
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_TextCoord[%d] vec2", uv);

                    renderer::VertexInputAttributeDesc::InputAttribute attrib;
                    attrib._binding = 0;
                    attrib._stream = 0;
                    attrib._format = renderer::Format::Format_R32G32_SFloat;
                    attrib._offset = vertexSize;
                    inputAttributes.push_back(attrib);

                    vertexSize += sizeof(math::float2);
                }
            }

            static const ModelFileLoader::VertexProperies colorFlag[k_maxVertexColorCount] =
            {
                ModelFileLoader::VertexProperies::VertexProperies_Color0,
                ModelFileLoader::VertexProperies::VertexProperies_Color1,
                ModelFileLoader::VertexProperies::VertexProperies_Color2,
                ModelFileLoader::VertexProperies::VertexProperies_Color3
            };

            for (u32 c = 0; c < k_maxVertexColorCount; c++)
            {
                if (presentFlags & toEnumType(colorFlag[c]))
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Color[%d] vec2", c);

                    renderer::VertexInputAttributeDesc::InputAttribute attrib;
                    attrib._binding = 0;
                    attrib._stream = 0;
                    attrib._format = renderer::Format::Format_R32G32B32A32_SFloat;
                    attrib._offset = vertexSize;
                    inputAttributes.push_back(attrib);

                    vertexSize += sizeof(math::float4);
                }
            }

            return vertexSize;
        };

        const aiMesh* mesh = scene->mMeshes[m];
        std::string name = mesh->mName.C_Str();

        LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Load mesh index %d, name %s, material index %d", m, name.c_str(), mesh->mMaterialIndex);
        ASSERT((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != 0, "must be triangle");

        modelStream->write<u32>(1); //Assimp doesn't support LODs loading, always one
        streamMeshesSize += sizeof(u32);

        u64 meshStreamSize = 0;
        stream::Stream* meshStream = stream::StreamManager::createMemoryStream();

        meshStreamSize += meshStream->write(name);

        //Calculate size of vertex
        u32 bindingIndex = 0;
        renderer::VertexInputAttributeDesc attribDescription;
        if (flags & ModelFileLoader::SeperatePositionStream)
        {
            u32 stride = buildVertexData(mesh, toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Position));
            ASSERT(stride > 0, "invalid stride");
            u32 meshBufferSize = stride * mesh->mNumVertices;
            attribDescription._inputBindings[attribDescription._countInputBindings++] = renderer::VertexInputAttributeDesc::InputBinding(bindingIndex, renderer::InputRate::InputRate_Vertex, stride);

            vertexPropFlags |= ~toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Position);
            ++bindingIndex;
        }
        u32 stride = buildVertexData(mesh, vertexPropFlags);
        ASSERT(stride > 0, "invalid stride");
        u32 meshBufferSize = stride * mesh->mNumVertices;
        attribDescription._inputBindings[attribDescription._countInputBindings++] = renderer::VertexInputAttributeDesc::InputBinding(bindingIndex, renderer::InputRate::InputRate_Vertex, stride);

        memcpy(attribDescription._inputAttributes.data(), inputAttributes.data(), inputAttributes.size() * sizeof(renderer::VertexInputAttributeDesc::InputAttribute));
        attribDescription._countInputAttributes = static_cast<u32>(inputAttributes.size());

        meshStreamSize += attribDescription >> meshStream;

        //count streams per mesh
        meshStream->write<u32>(attribDescription._countInputBindings);
        meshStreamSize += sizeof(u32);

        //Fill data of a vertex
        if (flags & ModelFileLoader::SeperatePositionStream)
        {
            u32 stride = buildVertexData(mesh, toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Position));
            ASSERT(stride > 0, "invalid stride");
            u32 meshBufferSize = stride * mesh->mNumVertices;

            meshStream->write<u32>(meshBufferSize);
            meshStreamSize += sizeof(u32);

            if(mesh->HasPositions())
            {
                for (u32 v = 0; v < mesh->mNumVertices; v++)
                {
                    math::float3 position;
                    position._x = mesh->mVertices[v].x;
                    position._y = (flags & ModelFileLoader::FlipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                    position._z = mesh->mVertices[v].z;

                    meshStream->write<math::float3>(position);
                    meshStreamSize += sizeof(math::float3);
                }
            }
        }
        meshStream->write<u32>(mesh->mNumVertices);
        meshStreamSize += sizeof(u32);

        meshStream->write<u32>(meshBufferSize);
        meshStreamSize += sizeof(u32);

        for (u32 v = 0; v < mesh->mNumVertices; v++)
        {
            if (vertexPropFlags & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Position))
            {
                ASSERT(mesh->HasPositions(), "must be presented");
                math::float3 position;
                position._x = mesh->mVertices[v].x;
                position._y = (flags & ModelFileLoader::FlipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                position._z = mesh->mVertices[v].z;

                meshStream->write<math::float3>(position);
                meshStreamSize += sizeof(math::float3);
            }

            if (vertexPropFlags & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Normals))
            {
                ASSERT(mesh->HasNormals(), "must be presented");
                math::float3 normal;
                normal._x = mesh->mNormals[v].x;
                normal._y = mesh->mNormals[v].y;
                normal._z = mesh->mNormals[v].z;

                meshStream->write<math::float3>(normal);
                meshStreamSize += sizeof(math::float3);
            }

            if (vertexPropFlags & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Tangent))
            {
                ASSERT(mesh->HasTangentsAndBitangents(), "must be presented");
                math::float3 tangent;
                tangent._x = mesh->mTangents[v].x;
                tangent._y = mesh->mTangents[v].y;
                tangent._z = mesh->mTangents[v].z;

                meshStream->write<math::float3>(tangent);
                meshStreamSize += sizeof(math::float3);
            }

            if (vertexPropFlags & toEnumType(ModelFileLoader::VertexProperies::VertexProperies_Bitangent))
            {
                ASSERT(mesh->HasTangentsAndBitangents(), "must be presented");
                math::float3 bitangent;
                bitangent._x = mesh->mBitangents[v].x;
                bitangent._y = mesh->mBitangents[v].y;
                bitangent._z = mesh->mBitangents[v].z;

                meshStream->write<math::float3>(bitangent);
                meshStreamSize += sizeof(math::float3);
            }


            static const ModelFileLoader::VertexProperies uvFlag[k_maxTextureCoordsCount] =
            {
                ModelFileLoader::VertexProperies::VertexProperies_TextCoord0,
                ModelFileLoader::VertexProperies::VertexProperies_TextCoord1,
                ModelFileLoader::VertexProperies::VertexProperies_TextCoord2,
                ModelFileLoader::VertexProperies::VertexProperies_TextCoord3
            };

            for (u32 uv = 0; uv < resource::k_maxTextureCoordsCount; uv++)
            {
                if (vertexPropFlags & toEnumType(uvFlag[uv]))
                {
                    math::float2 coord;
                    if (mesh->HasTextureCoords(uv))
                    {
                        coord._x = mesh->mTextureCoords[uv][v].x;
                        coord._y = (flags & ModelFileLoader::FlipYTextureCoord) ? -mesh->mTextureCoords[uv][v].y : mesh->mTextureCoords[uv][v].y;
                    }
                    else
                    {
                        coord._x = 0.f;
                        coord._y = 0.f;
                    }
                    meshStream->write<math::float2>(coord);
                    meshStreamSize += sizeof(math::float2);
                }
            }


            static const ModelFileLoader::VertexProperies colorFlag[k_maxVertexColorCount] =
            {
                ModelFileLoader::VertexProperies::VertexProperies_Color0,
                ModelFileLoader::VertexProperies::VertexProperies_Color1,
                ModelFileLoader::VertexProperies::VertexProperies_Color2,
                ModelFileLoader::VertexProperies::VertexProperies_Color3
            };

            for (u32 c = 0; c < resource::k_maxVertexColorCount; c++)
            {
                if (vertexPropFlags & toEnumType(colorFlag[c]))
                {
                    math::float4 color;
                    if (mesh->HasVertexColors(c))
                    {
                        color._x = mesh->mColors[c][v].r;
                        color._y = mesh->mColors[c][v].g;
                        color._z = mesh->mColors[c][v].b;
                        color._w = mesh->mColors[c][v].a;
                    }
                    else
                    {
                        color = { 0.f, 0.f, 0.f, 1.f };
                    }

                    meshStream->write<math::float4>(color);
                    meshStreamSize += sizeof(math::float4);
                }
            }
        }
#ifdef DEBUG
        ASSERT(meshStreamSize == meshStream->size(), "different sizes");
#endif //DEBUG

        std::vector<u32> index32Buffer;
        if (!(flags & ModelFileLoader::SkipIndexBuffer))
        {
            for (u32 f = 0; f < scene->mMeshes[m]->mNumFaces; f++)
            {
                const aiFace& face = scene->mMeshes[m]->mFaces[f];
                for (u32 i = 0; i < face.mNumIndices; i++)
                {
                    index32Buffer.push_back(face.mIndices[i]);
                }
            }

            meshStream->write<u32>(static_cast<u32>(index32Buffer.size()));
            meshStream->write<bool>(true); //is 32bit type
            meshStreamSize += sizeof(u32) + sizeof(bool);

            u32 indexBufferSize = static_cast<u32>(index32Buffer.size()) * sizeof(u32);
            meshStream->write(index32Buffer.data(), indexBufferSize, 1);
            meshStreamSize += indexBufferSize;
        }
        else
        {
            meshStream->write<u32>(0);
            meshStreamSize += sizeof(u32);
        }

        math::TVector3D<f32> min(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z);
        math::TVector3D<f32> max(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
        math::AABB aabb(min, max);

        meshStream->write<math::AABB>(aabb);
        meshStreamSize += sizeof(math::AABB);

#ifdef DEBUG
        ASSERT(meshStreamSize == meshStream->size(), "different sizes");
#endif //DEBUG

        meshStream->seekBeg(0);
        modelStream->write(meshStream->map(meshStream->size()), meshStream->size());
        streamMeshesSize += meshStream->size();
        meshStream->unmap();
        stream::StreamManager::destroyStream(meshStream);

        m_materialMapper.push_back({ m, mesh->mMaterialIndex });
    }

    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: load meshes: %d, size %d bytes", scene->mNumMeshes, streamMeshesSize);
    return streamMeshesSize;
}

u32 AssimpDecoder::decodeSkeleton(const aiScene* scene, stream::Stream* stream) const
{
    return u32();
}

u32 AssimpDecoder::decodeMaterial(const aiScene* scene, stream::Stream* modelStream) const
{
    ASSERT(modelStream, "nullptr");

    enum PropertyType : u8
    {
        Scalar = 0,
        Vector = 1,
        Texture = 2
    };

    auto parseUnknownProperty = [](aiMaterial* material, aiMaterialProperty* propery)
    {
        switch (propery->mType)
        {
        case aiPTI_Float:
        {
            f32 value;
            aiReturn result = material->Get(propery->mKey.C_Str(), 0, 0, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            break;
        }

        case aiPTI_Double:
        {
            f64 value;
            aiReturn result = material->Get(propery->mKey.C_Str(), 0, 0, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            break;
        }
        case aiPTI_String:
        {
            aiString value;
            aiReturn result = material->Get(propery->mKey.C_Str(), 0, 0, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            break;
        }
        case aiPTI_Integer:
        {
            s32 value;
            aiReturn result = material->Get(propery->mKey.C_Str(), 0, 0, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            break;
        }
        case aiPTI_Buffer:
        default:
            ASSERT(false, "not handle");
        }
    };

    auto parseProperty = [&parseUnknownProperty](aiMaterial* material, aiMaterialProperty* materialProperty, u32 id, stream::Stream* stream) -> bool
    {
        if (materialProperty->mKey == aiString("$mat.twosided"))
        {
            ASSERT(materialProperty->mType == aiPTI_Integer, "wrong type");
            s32 value;
            aiReturn result = material->Get(AI_MATKEY_TWOSIDED, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Twosided";
            u8 type = PropertyType::Scalar;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.shadingm"))
        {
            if (materialProperty->mType == aiPTI_Integer)
            {
                s32 value;
                aiReturn result = material->Get(AI_MATKEY_SHADING_MODEL, value);
                ASSERT(result == aiReturn_SUCCESS, "can't read");

                aiShadingMode model = (aiShadingMode)value;

                return false;
            }
            else if (materialProperty->mType == aiPTI_Buffer && materialProperty->mDataLength == sizeof(u32))
            {
                u32 value;
                aiReturn result = material->Get(AI_MATKEY_SHADING_MODEL, value);
                ASSERT(result == aiReturn_SUCCESS, "can't read");

                aiShadingMode model = (aiShadingMode)value;

                return false;
            }

            ASSERT(false, "wrong type");
            return false;
        }
        else if (materialProperty->mKey == aiString("$mat.wireframe"))
        {
            ASSERT(materialProperty->mType == aiPTI_Integer, "wrong type");
            s32 value;
            aiReturn result = material->Get(AI_MATKEY_ENABLE_WIREFRAME, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            bool isWireframe = value;

            return false;
        }
        else if (materialProperty->mKey == aiString("$mat.blend"))
        {
            ASSERT(materialProperty->mType == aiPTI_Integer, "wrong type");
            s32 value;
            aiReturn result = material->Get(AI_MATKEY_BLEND_FUNC, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            return false;
        }
        else if (materialProperty->mKey == aiString("$mat.opacity"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_OPACITY, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Opacity";
            u8 type = PropertyType::Scalar;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.transparencyfactor"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_TRANSPARENCYFACTOR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Transparent";
            u8 type = PropertyType::Scalar;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.bumpscaling"))
        {
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_BUMPSCALING, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Bump";
            u8 type = PropertyType::Scalar;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.shininess"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_SHININESS, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Shininess";
            u8 type = PropertyType::Scalar;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.reflectivity"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_REFLECTIVITY, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Reflectivity";
            u8 type = PropertyType::Scalar;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.shinpercent"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_SHININESS_STRENGTH, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Shininess";
            u8 type = PropertyType::Scalar;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.refracti"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_REFRACTI, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Refract";
            u8 type = PropertyType::Scalar;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.diffuse"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_DIFFUSE, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "DiffuseColor";
            u8 type = PropertyType::Vector;
            math::float4 color;
            color._x = value.r;
            color._y = value.g;
            color._z = value.b;
            color._w = value.a;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<math::float4>(color);

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.ambient"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_AMBIENT, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "AmbientColor";
            u8 type = PropertyType::Vector;
            math::float4 color;
            color._x = value.r;
            color._y = value.g;
            color._z = value.b;
            color._w = value.a;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<math::float4>(color);

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.specular"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_SPECULAR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "SpecularColor";
            u8 type = PropertyType::Vector;
            math::float4 color;
            color._x = value.r;
            color._y = value.g;
            color._z = value.b;
            color._w = value.a;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<math::float4>(color);

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.emissive"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_EMISSIVE, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "EmissiveColor";
            u8 type = PropertyType::Vector;
            math::float4 color;
            color._x = value.r;
            color._y = value.g;
            color._z = value.b;
            color._w = value.a;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<math::float4>(color);

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.transparent"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_TRANSPARENT, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "TransparentColor";
            u8 type = PropertyType::Vector;
            math::float4 color;
            color._x = value.r;
            color._y = value.g;
            color._z = value.b;
            color._w = value.a;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<math::float4>(color);

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.reflective"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_REFLECTIVE, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "ReflectiveColor";
            u8 type = PropertyType::Vector;
            math::float4 color;
            color._x = value.r;
            color._y = value.g;
            color._z = value.b;
            color._w = value.a;

            stream->write(name);
            stream->write<u8>(type);
            stream->write<math::float4>(color);

            return true;
        }
        else
        {
            ASSERT(false, "unhandle");
            parseUnknownProperty(material, materialProperty);
        }

        return false;
    };

    const std::map<std::string, std::function<bool(aiMaterial*, aiMaterialProperty*, u32, stream::Stream*)>> vectorProperties =
    {
        //{ "?mat.name", parseProperty },
        { "$mat.twosided", parseProperty },
        { "$mat.shadingm", parseProperty },
        { "$mat.wireframe", parseProperty },
        { "$mat.blend", parseProperty },
        { "$mat.opacity", parseProperty },
        { "$mat.transparencyfactor", parseProperty },
        { "$mat.bumpscaling",parseProperty },
        { "$mat.shininess", parseProperty },
        { "$mat.reflectivity", parseProperty },
        { "$mat.shinpercent", parseProperty },
        { "$mat.refracti", parseProperty },
        { "$clr.diffuse", parseProperty },
        { "$clr.ambient", parseProperty },
        { "$clr.specular",parseProperty },
        { "$clr.emissive", parseProperty },
        { "$clr.transparent", parseProperty },
        { "$clr.reflective",parseProperty },
        { "?bg.global", parseProperty },
        { "?sh.lang", parseProperty },
        { "?sh.vs", parseProperty },
        { "?sh.fs", parseProperty },
        { "?sh.gs", parseProperty },
        { "?sh.ts", parseProperty },
        { "?sh.ps", parseProperty },
        { "?sh.cs", parseProperty },
    };

    modelStream->write<u32>(scene->mNumMaterials);
    u32 streamMaterialsSize = sizeof(u32);

    u32 uid = 0;
    std::map<u32, std::string> imageList;
    for (u32 m = 0; m < scene->mNumMaterials; m++)
    {
        stream::Stream* materialStream = stream::StreamManager::createMemoryStream();
        u32 propertiesCount = 0;

        aiMaterial* material = scene->mMaterials[m];
        std::string name = std::string(material->GetName().C_Str());

        for (u32 p = 0; p < material->mNumProperties; ++p)
        {
            aiMaterialProperty* property = material->mProperties[p];

            auto found = vectorProperties.find(property->mKey.C_Str());
            if (found != vectorProperties.cend())
            {
                if (found->second(material, property, p, materialStream))
                {
                    ++propertiesCount;
                }
            }
        }

        auto texturePropertyName = [](aiTextureType type) -> std::string
        {
            switch (type)
            {
            case aiTextureType::aiTextureType_DIFFUSE:
                return "Diffuse";
            case aiTextureType::aiTextureType_SPECULAR:
                return "Specular";
            case aiTextureType::aiTextureType_AMBIENT:
                return "Ambient";
            case aiTextureType::aiTextureType_EMISSIVE:
                return "Emission";
            case aiTextureType::aiTextureType_HEIGHT:
                return "Height";
            case aiTextureType::aiTextureType_NORMALS:
                return "Normal";
            case aiTextureType::aiTextureType_SHININESS:
                return "Shininess";
            case aiTextureType::aiTextureType_OPACITY:
                return "Opacity";
            case aiTextureType::aiTextureType_DISPLACEMENT:
                return "Displacement";
            case aiTextureType::aiTextureType_LIGHTMAP:
                return "Lightmap";
            case aiTextureType::aiTextureType_REFLECTION:
                return "Reflection";
            case aiTextureType::aiTextureType_BASE_COLOR:
                return "BaseColor";
            case aiTextureType::aiTextureType_NORMAL_CAMERA:
                return "Normal";
            case aiTextureType::aiTextureType_EMISSION_COLOR:
                return "Emission";
            case aiTextureType::aiTextureType_METALNESS:
                return "Metalness";
            case aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS:
                return "Roughness";
            case aiTextureType::aiTextureType_AMBIENT_OCCLUSION:
                return "AmbientOcclusion";

            case aiTextureType::aiTextureType_NONE:
            default:
                ASSERT(false, "not found");
                return "Unknown";
            }

            return "Unknown";
        };

        for (u32 type = aiTextureType::aiTextureType_NONE; type <= aiTextureType::aiTextureType_UNKNOWN; ++type)
        {
            u32 countTextures = material->GetTextureCount((aiTextureType)type);
            for (u32 t = 0; t < countTextures; ++t)
            {
                aiString path;
                aiReturn result = material->GetTexture((aiTextureType)type, t, &path);
                ASSERT(result == aiReturn_SUCCESS, "can't read");

                std::string name = texturePropertyName((aiTextureType)type);
                u8 type = PropertyType::Texture;
                u32 index = uid++;

                materialStream->write(name);
                materialStream->write<u8>(type);
                materialStream->write<u32>(index);
                ++propertiesCount;

                imageList.emplace(index, path.C_Str());
            }
        }

        streamMaterialsSize += modelStream->write(name);
        modelStream->write<u32>(propertiesCount);
        streamMaterialsSize += sizeof(u32);

        materialStream->seekBeg(0);
        modelStream->write(materialStream->map(materialStream->size()), materialStream->size());
        streamMaterialsSize += materialStream->size();

        materialStream->unmap();
        stream::StreamManager::destroyStream(materialStream);

        LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: Load material[%d] name %s", m, name.c_str());
    }

    modelStream->write<u32>(static_cast<u32>(imageList.size()));
    for (auto& [id, textureName] : imageList)
    {
        streamMaterialsSize += modelStream->write(textureName);
    };

    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: load materials: %d, size %d bytes", scene->mNumMaterials, streamMaterialsSize);
    return streamMaterialsSize;
}

u32 AssimpDecoder::decodeLight(const aiScene* scene, stream::Stream* stream) const
{
    for (u32 i = 0; i < scene->mNumLights; ++i)
    {
        aiLight* light = scene->mLights[i];
        std::string name = light->mName.C_Str();

        //TODO
    }

    return 0;
}

u32 AssimpDecoder::decodeCamera(const aiScene* scene, stream::Stream* stream) const
{
    return u32();
}

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP
