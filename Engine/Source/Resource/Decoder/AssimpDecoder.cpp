#include "AssimpDecoder.h"

#include "Renderer/Formats.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Loader//ModelFileLoader.h"

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

        u32 assimpFlags = 0;
        const aiScene* scene;
        Assimp::Importer Importer;

        MeshResource::VertexProperiesFlags activeFlags = MeshResource::VertexProperies::VertexProperies_Position;
        assimpFlags = aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_SortByPType;

        if (!(flags & ModelFileLoader::SkipIndexBuffer))
        {
            assimpFlags |= aiProcess_JoinIdenticalVertices;
        }

        if (!(flags & ModelFileLoader::SkipNormals))
        {
            assimpFlags |= aiProcess_GenSmoothNormals;
            activeFlags |= MeshResource::VertexProperies::VertexProperies_Normals;
        }

        if (!(flags & ModelFileLoader::SkipTangentAndBitangent))
        {
            assimpFlags |= aiProcess_CalcTangentSpace;
            activeFlags |= MeshResource::VertexProperies::VertexProperies_Tangent | MeshResource::VertexProperies::VertexProperies_Bitangent;
        }

        if (!(flags & ModelFileLoader::SkipTextureCoord))
        {
            assimpFlags |= aiProcess_GenUVCoords;
            activeFlags |= MeshResource::VertexProperies::VertexProperies_TextCoord0 | MeshResource::VertexProperies::VertexProperies_TextCoord1 |
                MeshResource::VertexProperies::VertexProperies_TextCoord2 | MeshResource::VertexProperies::VertexProperies_TextCoord3;
        }

        if (flags & ModelFileLoader::GenerateBoundingBoxes)
        {
            assimpFlags |= aiProcess_GenBoundingBoxes;
        }

        if (flags & ModelFileLoader::SplitLargeMeshes)
        {
            assimpFlags |= aiProcess_SplitLargeMeshes;
        }

        if (!(flags & ModelFileLoader::LocalTransform))
        {
            assimpFlags |= aiProcess_PreTransformVertices;
        }

        u8* data = stream->map(stream->size());
        ASSERT(data, "nullptr");
        scene = Importer.ReadFileFromMemory(data, stream->size(), assimpFlags);
        stream->unmap();
        if (!scene)
        {
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

        u32 modelStreamSize = 0;
        stream::Stream* modelStream = stream::StreamManager::createMemoryStream();

        modelStream->write<ModelResource::ModelContentFlags>(contentFlags);
        if (contentFlags & ModelResource::ModelContent_Meshes)
        {
            modelStreamSize += AssimpDecoder::decodeMesh(scene, modelStream, flags, activeFlags);
        }

        if (contentFlags & ModelResource::ModelContent_Materials)
        {
            modelStreamSize += AssimpDecoder::decodeMaterial(scene, modelStream);
            contentFlags |= ModelResource::ModelContent_Materials;
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
        MeshResource::VertexProperiesFlags contentFlag = 0;
        std::vector<renderer::VertexInputAttributeDesc::InputAttribute> inputAttributes;
        static auto buildVertexData = [&inputAttributes, &contentFlag](const aiMesh* mesh, MeshResource::VertexProperiesFlags presentFlags) -> u32
        {
            u32 vertexSize = 0;
            if (mesh->HasPositions() && (presentFlags & MeshResource::VertexProperies::VertexProperies_Position))
            {
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Position vec3");

                renderer::VertexInputAttributeDesc::InputAttribute attrib;
                attrib._binding = 0;
                attrib._stream = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offset = vertexSize;
                inputAttributes.push_back(attrib);

                contentFlag |= resource::MeshResource::VertexProperies::VertexProperies_Position;
                vertexSize += sizeof(math::TVector3D<f32>);
            }

            if (mesh->HasNormals() && (presentFlags & MeshResource::VertexProperies::VertexProperies_Normals))
            {
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Normals vec3");

                renderer::VertexInputAttributeDesc::InputAttribute attrib;
                attrib._binding = 0;
                attrib._stream = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offset = vertexSize;
                inputAttributes.push_back(attrib);

                contentFlag |= resource::MeshResource::VertexProperies::VertexProperies_Normals;
                vertexSize += sizeof(math::TVector3D<f32>);
            }

            if (mesh->HasTangentsAndBitangents() && 
                (presentFlags & (MeshResource::VertexProperies::VertexProperies_Tangent | MeshResource::VertexProperies::VertexProperies_Bitangent)))
            {
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Tangent vec3");

                    renderer::VertexInputAttributeDesc::InputAttribute attrib;
                    attrib._binding = 0;
                    attrib._stream = 0;
                    attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                    attrib._offset = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= resource::MeshResource::VertexProperies::VertexProperies_Tangent;
                    vertexSize += sizeof(math::TVector3D<f32>);
                }
                
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Bitangent vec3");

                    renderer::VertexInputAttributeDesc::InputAttribute attrib;
                    attrib._binding = 0;
                    attrib._stream = 0;
                    attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                    attrib._offset = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= MeshResource::VertexProperies::VertexProperies_Bitangent;
                    vertexSize += sizeof(math::TVector3D<f32>);
                }
            }

            for (u32 uv = 0; uv < k_maxTextureCoordsCount; uv++)
            {
                if (mesh->HasTextureCoords(uv) && (presentFlags & MeshResource::VertexProperies::VertexProperies_TextCoord0 + uv))
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_TextCoord[%d] vec2", uv);

                    renderer::VertexInputAttributeDesc::InputAttribute attrib;
                    attrib._binding = 0;
                    attrib._stream = 0;
                    attrib._format = renderer::Format::Format_R32G32_SFloat;
                    attrib._offset = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= MeshResource::VertexProperies::VertexProperies_TextCoord0 + uv;
                    vertexSize += sizeof(math::TVector2D<f32>);
                }
            }

            for (u32 c = 0; c < k_maxVertexColorCount; c++)
            {
                if (mesh->HasVertexColors(c) && (presentFlags & MeshResource::VertexProperies::VertexProperies_Color0 + c))
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Color[%d] vec2", c);

                    renderer::VertexInputAttributeDesc::InputAttribute attrib;
                    attrib._binding = 0;
                    attrib._stream = 0;
                    attrib._format = renderer::Format::Format_R32G32B32A32_SFloat;
                    attrib._offset = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= MeshResource::VertexProperies::VertexProperies_Color0 + c;
                    vertexSize += sizeof(math::TVector4D<f32>);
                }
            }

            return vertexSize;
        };

        const aiMesh* mesh = scene->mMeshes[m];
        LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Load mesh index %d, name %s, material index %d", m, mesh->mName.C_Str(), mesh->mMaterialIndex);
        ASSERT((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != 0, "must be triangle");

        modelStream->write<u32>(1); //Assimp doesn't support LODs loading, always one
        streamMeshesSize += sizeof(u32);

        u64 meshStreamSize = 0;
        stream::Stream* meshStream = stream::StreamManager::createMemoryStream();

        //Calculate size of vertex
        u32 bindingIndex = 0;
        renderer::VertexInputAttributeDesc attribDescription;
        if (flags & ModelFileLoader::SeperatePositionStream)
        {
            u32 stride = buildVertexData(mesh, resource::MeshResource::VertexProperies::VertexProperies_Position);
            ASSERT(stride > 0, "invalid stride");
            u32 meshBufferSize = stride * mesh->mNumVertices;
            attribDescription._inputBindings[attribDescription._countInputBindings++] = renderer::VertexInputAttributeDesc::InputBinding(bindingIndex, renderer::InputRate::InputRate_Vertex, stride);

            vertexPropFlags |= ~MeshResource::VertexProperies::VertexProperies_Position;
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
            u32 stride = buildVertexData(mesh, resource::MeshResource::VertexProperies::VertexProperies_Position);
            ASSERT(stride > 0, "invalid stride");
            u32 meshBufferSize = stride * mesh->mNumVertices;

            meshStream->write<u32>(meshBufferSize);
            meshStreamSize += sizeof(u32);

            if(mesh->HasPositions())
            {
                for (u32 v = 0; v < mesh->mNumVertices; v++)
                {
                    math::TVector3D<f32> position;
                    position._x = mesh->mVertices[v].x;
                    position._y = (flags & ModelFileLoader::FlipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                    position._z = mesh->mVertices[v].z;

                    meshStream->write<math::TVector3D<f32>>(position);
                    meshStreamSize += sizeof(math::TVector3D<f32>);
                }
            }
        }
        meshStream->write<u32>(mesh->mNumVertices);
        meshStreamSize += sizeof(u32);

        meshStream->write<u32>(meshBufferSize);
        meshStreamSize += sizeof(u32);

        for (u32 v = 0; v < mesh->mNumVertices; v++)
        {
            if (mesh->HasPositions() && (vertexPropFlags & MeshResource::VertexProperies::VertexProperies_Position))
            {
                math::TVector3D<f32> position;
                position._x = mesh->mVertices[v].x;
                position._y = (flags & ModelFileLoader::FlipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                position._z = mesh->mVertices[v].z;

                meshStream->write<math::TVector3D<f32>>(position);
                meshStreamSize += sizeof(math::TVector3D<f32>);
            }

            if (mesh->HasNormals() && (vertexPropFlags & MeshResource::VertexProperies::VertexProperies_Normals))
            {
                math::TVector3D<f32> normal;
                normal._x = mesh->mNormals[v].x;
                normal._y = mesh->mNormals[v].y;
                normal._z = mesh->mNormals[v].z;

                meshStream->write<math::TVector3D<f32>>(normal);
                meshStreamSize += sizeof(math::TVector3D<f32>);
            }

            if (mesh->HasTangentsAndBitangents() && 
                (vertexPropFlags & (MeshResource::VertexProperies::VertexProperies_Tangent | MeshResource::VertexProperies::VertexProperies_Bitangent)))
            {
                math::TVector3D<f32> tangent;
                tangent._x = mesh->mTangents[v].x;
                tangent._y = mesh->mTangents[v].y;
                tangent._z = mesh->mTangents[v].z;

                meshStream->write<math::TVector3D<f32>>(tangent);
                meshStreamSize += sizeof(math::TVector3D<f32>);

                math::TVector3D<f32> bitangent;
                bitangent._x = mesh->mBitangents[v].x;
                bitangent._y = mesh->mBitangents[v].y;
                bitangent._z = mesh->mBitangents[v].z;

                meshStream->write<math::TVector3D<f32>>(bitangent);
                meshStreamSize += sizeof(math::TVector3D<f32>);
            }

            for (u32 uv = 0; uv < resource::k_maxTextureCoordsCount; uv++)
            {
                if (mesh->HasTextureCoords(uv) && (vertexPropFlags & MeshResource::VertexProperies::VertexProperies_TextCoord0 + uv))
                {
                    math::TVector2D<f32> coord;
                    coord._x = mesh->mTextureCoords[uv][v].x;
                    coord._y = (flags & ModelFileLoader::FlipYTextureCoord) ? -mesh->mTextureCoords[uv][v].y : mesh->mTextureCoords[uv][v].y;

                    meshStream->write<math::TVector2D<f32>>(coord);
                    meshStreamSize += sizeof(math::TVector2D<f32>);
                }
            }

            for (u32 c = 0; c < resource::k_maxVertexColorCount; c++)
            {
                if (mesh->HasVertexColors(c) && (vertexPropFlags & MeshResource::VertexProperies::VertexProperies_Color0 + c))
                {
                    math::TVector4D<f32> color;
                    color._x = mesh->mColors[c][v].r;
                    color._y = mesh->mColors[c][v].g;
                    color._z = mesh->mColors[c][v].b;
                    color._w = mesh->mColors[c][v].a;

                    meshStream->write<math::TVector4D<f32>>(color);
                    meshStreamSize += sizeof(math::TVector4D<f32>);
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
#ifdef DEBUG
        ASSERT(meshStreamSize == meshStream->size(), "different sizes");
#endif //DEBUG

        if (flags & ModelFileLoader::GenerateBoundingBoxes)
        {
            math::TVector3D<f32> min(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z);
            math::TVector3D<f32> max(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
            math::AABB aabb(min, max);

            meshStream->write<math::AABB>(aabb);
            meshStreamSize += sizeof(math::AABB);
        }
#ifdef DEBUG
        ASSERT(meshStreamSize == meshStream->size(), "different sizes");
#endif //DEBUG

        resource::MeshResource::MeshHeader meshHeader;
        resource::ResourceHeader::fill(&meshHeader, mesh->mName.C_Str(), meshStream->size(), modelStream->size() + sizeof(resource::MeshResource::MeshHeader));

        streamMeshesSize += meshHeader >> modelStream;

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

u32 AssimpDecoder::decodeMaterial(const aiScene* scene, stream::Stream* modelStream) const
{
    ASSERT(modelStream, "nullptr");

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

    //auto parseProperty = [&parseUnknownProperty](aiMaterial* material, aiMaterialProperty* materialProperty, u32 id, stream::Stream* stream) -> bool
    //{
    //    scene::Material::Property property;
    //    property._index = id;
    //    property._array = 1;

    //    if (materialProperty->mKey == aiString("$mat.twosided"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Integer, "wrong type");
    //        s32 value;
    //        aiReturn result = material->Get(AI_MATKEY_TWOSIDED, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        bool twoSided = value;

    //        return false;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.shadingm"))
    //    {
    //        if (materialProperty->mType == aiPTI_Integer)
    //        {
    //            s32 value;
    //            aiReturn result = material->Get(AI_MATKEY_SHADING_MODEL, value);
    //            ASSERT(result == aiReturn_SUCCESS, "can't read");

    //            aiShadingMode model = (aiShadingMode)value;

    //            return false;
    //        }
    //        else if (materialProperty->mType == aiPTI_Buffer)
    //        {
    //            if (materialProperty->mDataLength == sizeof(u32))
    //            {
    //                u32 value;
    //                aiReturn result = material->Get(AI_MATKEY_SHADING_MODEL, value);
    //                ASSERT(result == aiReturn_SUCCESS, "can't read");

    //                aiShadingMode model = (aiShadingMode)value;

    //                return false;
    //            }
    //        }

    //        ASSERT(false, "wrong type");
    //        return false;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.wireframe"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Integer, "wrong type");
    //        s32 value;
    //        aiReturn result = material->Get(AI_MATKEY_ENABLE_WIREFRAME, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        bool isWireframe = value;

    //        return false;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.blend"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Integer, "wrong type");
    //        s32 value;
    //        aiReturn result = material->Get(AI_MATKEY_BLEND_FUNC, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        //TODO

    //        return false;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.opacity"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        f32 value;
    //        aiReturn result = material->Get(AI_MATKEY_OPACITY, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        property._label = scene::Material::Property_Opacity;
    //        property._type = scene::Material::PropertyType::Value;
    //        property._data = scene::Material::Property::ValueProperty{ value };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.transparencyfactor"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        f32 value;
    //        aiReturn result = material->Get(AI_MATKEY_TRANSPARENCYFACTOR, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        property._label = scene::Material::Property_Transparent;
    //        property._type = scene::Material::PropertyType::Value;
    //        property._data = scene::Material::Property::ValueProperty{ value };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.bumpscaling"))
    //    {
    //        f32 value;
    //        aiReturn result = material->Get(AI_MATKEY_BUMPSCALING, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        property._label = scene::Material::Property_Bump;
    //        property._type = scene::Material::PropertyType::Value;
    //        property._data = scene::Material::Property::ValueProperty{ value };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.shininess"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        f32 value;
    //        aiReturn result = material->Get(AI_MATKEY_SHININESS, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        property._label = scene::Material::Property_Shininess;
    //        property._type = scene::Material::PropertyType::Value;
    //        property._data = scene::Material::Property::ValueProperty{ value };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.reflectivity"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        f32 value;
    //        aiReturn result = material->Get(AI_MATKEY_REFLECTIVITY, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        property._label = scene::Material::Property_Reflection;
    //        property._type = scene::Material::PropertyType::Value;
    //        property._data = scene::Material::Property::ValueProperty{ value };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.shinpercent"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        f32 value;
    //        aiReturn result = material->Get(AI_MATKEY_SHININESS_STRENGTH, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        property._label = scene::Material::Property_Shininess;
    //        property._type = scene::Material::PropertyType::Value;
    //        property._data = scene::Material::Property::ValueProperty{ value };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$mat.refracti"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        f32 value;
    //        aiReturn result = material->Get(AI_MATKEY_REFRACTI, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        property._label = scene::Material::Property_Refract;
    //        property._type = scene::Material::PropertyType::Value;
    //        property._data = scene::Material::Property::ValueProperty{ value };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$clr.diffuse"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        aiColor4D value;
    //        aiReturn result = material->Get(AI_MATKEY_COLOR_DIFFUSE, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        math::TVector4D<f32> color;
    //        color.m_x = value.r;
    //        color.m_y = value.g;
    //        color.m_z = value.b;
    //        color.m_w = value.a;

    //        property._label = scene::Material::Property_Diffuse;
    //        property._type = scene::Material::PropertyType::Vector;
    //        property._data = scene::Material::Property::VectorProperty{ color };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$clr.ambient"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        aiColor4D value;
    //        aiReturn result = material->Get(AI_MATKEY_COLOR_AMBIENT, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        math::TVector4D<f32> color;
    //        color.m_x = value.r;
    //        color.m_y = value.g;
    //        color.m_z = value.b;
    //        color.m_w = value.a;

    //        property._label = scene::Material::Property_Ambient;
    //        property._type = scene::Material::PropertyType::Vector;
    //        property._data = scene::Material::Property::VectorProperty{ color };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$clr.specular"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        aiColor4D value;
    //        aiReturn result = material->Get(AI_MATKEY_COLOR_SPECULAR, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        math::TVector4D<f32> color;
    //        color.m_x = value.r;
    //        color.m_y = value.g;
    //        color.m_z = value.b;
    //        color.m_w = value.a;

    //        property._label = scene::Material::Property_Specular;
    //        property._type = scene::Material::PropertyType::Vector;
    //        property._data = scene::Material::Property::VectorProperty{ color };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$clr.emissive"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        aiColor4D value;
    //        aiReturn result = material->Get(AI_MATKEY_COLOR_EMISSIVE, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        math::TVector4D<f32> color;
    //        color.m_x = value.r;
    //        color.m_y = value.g;
    //        color.m_z = value.b;
    //        color.m_w = value.a;

    //        property._label = scene::Material::Property_Emission;
    //        property._type = scene::Material::PropertyType::Vector;
    //        property._data = scene::Material::Property::VectorProperty{ color };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$clr.transparent"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        aiColor4D value;
    //        aiReturn result = material->Get(AI_MATKEY_COLOR_TRANSPARENT, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        math::TVector4D<f32> color;
    //        color.m_x = value.r;
    //        color.m_y = value.g;
    //        color.m_z = value.b;
    //        color.m_w = value.a;

    //        property._label = scene::Material::Property_Transparent;
    //        property._type = scene::Material::PropertyType::Vector;
    //        property._data = scene::Material::Property::VectorProperty{ color };
    //        property >> stream;

    //        return true;
    //    }
    //    else if (materialProperty->mKey == aiString("$clr.reflective"))
    //    {
    //        ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
    //        aiColor4D value;
    //        aiReturn result = material->Get(AI_MATKEY_COLOR_REFLECTIVE, value);
    //        ASSERT(result == aiReturn_SUCCESS, "can't read");

    //        math::TVector4D<f32> color;
    //        color.m_x = value.r;
    //        color.m_y = value.g;
    //        color.m_z = value.b;
    //        color.m_w = value.a;

    //        property._label = scene::Material::Property_Reflection;
    //        property._type = scene::Material::PropertyType::Vector;
    //        property._data = scene::Material::Property::VectorProperty{ color };
    //        property >> stream;

    //        return true;
    //    }
    //    else
    //    {
    //        ASSERT(false, "unhandle");
    //        parseUnknownProperty(material, materialProperty);
    //    }

    //    return false;
    //};

    //const std::map<std::string, std::function<bool(aiMaterial*, aiMaterialProperty*, u32, stream::Stream*)>> vectorProperties =
    //{
    //    //{ "?mat.name", parseProperty },
    //    { "$mat.twosided", parseProperty },
    //    { "$mat.shadingm", parseProperty },
    //    { "$mat.wireframe", parseProperty },
    //    { "$mat.blend", parseProperty },
    //    { "$mat.opacity", parseProperty },
    //    { "$mat.transparencyfactor", parseProperty },
    //    { "$mat.bumpscaling",parseProperty },
    //    { "$mat.shininess", parseProperty },
    //    { "$mat.reflectivity", parseProperty },
    //    { "$mat.shinpercent", parseProperty },
    //    { "$mat.refracti", parseProperty },
    //    { "$clr.diffuse", parseProperty },
    //    { "$clr.ambient", parseProperty },
    //    { "$clr.specular",parseProperty },
    //    { "$clr.emissive", parseProperty },
    //    { "$clr.transparent", parseProperty },
    //    { "$clr.reflective",parseProperty },
    //    { "?bg.global", parseProperty },
    //    { "?sh.lang", parseProperty },
    //    { "?sh.vs", parseProperty },
    //    { "?sh.fs", parseProperty },
    //    { "?sh.gs", parseProperty },
    //    { "?sh.ts", parseProperty },
    //    { "?sh.ps", parseProperty },
    //    { "?sh.cs", parseProperty },
    //};

    //modelStream->write<u32>(scene->mNumMaterials);
    //u32 streamMaterialsSize = sizeof(u32);

    //for (u32 m = 0; m < scene->mNumMaterials; m++)
    //{
    //    stream::Stream* materialStream = stream::StreamManager::createMemoryStream();
    //    u32 propertiesCount = 0;

    //    aiMaterial* material = scene->mMaterials[m];

    //    for (u32 p = 0; p < material->mNumProperties; ++p)
    //    {
    //        aiMaterialProperty* property = material->mProperties[p];

    //        auto found = vectorProperties.find(property->mKey.C_Str());
    //        if (found != vectorProperties.cend())
    //        {
    //            if (found->second(material, property, p, materialStream))
    //            {
    //                ++propertiesCount;
    //            }
    //        }
    //    }

    //    auto texturePropertyName = [](aiTextureType type) -> scene::Material::PropertyName
    //    {
    //        switch (type)
    //        {
    //        case aiTextureType::aiTextureType_DIFFUSE:
    //            return scene::Material::PropertyName::Property_Diffuse;
    //        case aiTextureType::aiTextureType_SPECULAR:
    //            return scene::Material::PropertyName::Property_Specular;
    //        case aiTextureType::aiTextureType_AMBIENT:
    //            return scene::Material::PropertyName::Property_Ambient;
    //        case aiTextureType::aiTextureType_EMISSIVE:
    //            return scene::Material::PropertyName::Property_Emission;
    //        case aiTextureType::aiTextureType_HEIGHT:
    //            return scene::Material::PropertyName::Property_Height;
    //        case aiTextureType::aiTextureType_NORMALS:
    //            return scene::Material::PropertyName::Property_Normal;
    //        case aiTextureType::aiTextureType_SHININESS:
    //            return scene::Material::PropertyName::Property_Shininess;
    //        case aiTextureType::aiTextureType_OPACITY:
    //            return scene::Material::PropertyName::Property_Opacity;
    //        case aiTextureType::aiTextureType_DISPLACEMENT:
    //            return scene::Material::PropertyName::Property_Displacement;
    //        case aiTextureType::aiTextureType_LIGHTMAP:
    //            return scene::Material::PropertyName::Property_Light;
    //        case aiTextureType::aiTextureType_REFLECTION:
    //            return scene::Material::PropertyName::Property_Reflection;
    //        case aiTextureType::aiTextureType_BASE_COLOR:
    //            return scene::Material::PropertyName::Property_Albedo;
    //        case aiTextureType::aiTextureType_NORMAL_CAMERA:
    //            return scene::Material::PropertyName::Property_Normal;
    //        case aiTextureType::aiTextureType_EMISSION_COLOR:
    //            return scene::Material::PropertyName::Property_Emission;
    //        case aiTextureType::aiTextureType_METALNESS:
    //            return scene::Material::PropertyName::Property_Metallic;
    //        case aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS:
    //            return scene::Material::PropertyName::Property_Roughness;
    //        case aiTextureType::aiTextureType_AMBIENT_OCCLUSION:
    //            return scene::Material::PropertyName::Property_AmbientOcclusion;

    //        case aiTextureType::aiTextureType_NONE:
    //        default:
    //            ASSERT(false, "not found");
    //            return scene::Material::PropertyName::Property_Unknown;
    //        }

    //        return scene::Material::PropertyName::Property_Unknown;
    //    };

    //    for (u32 type = aiTextureType::aiTextureType_NONE; type <= aiTextureType::aiTextureType_UNKNOWN; ++type)
    //    {
    //        u32 countTextures = material->GetTextureCount((aiTextureType)type);
    //        for (u32 t = 0; t < countTextures; ++t)
    //        {
    //            u32 textureID = material->mNumProperties + type + t;

    //            aiString path;
    //            aiReturn result = material->GetTexture((aiTextureType)type, t, &path);
    //            ASSERT(result == aiReturn_SUCCESS, "can't read");

    //            scene::Material::Property property;
    //            property._index = textureID;
    //            property._array = 1;
    //            property._label = texturePropertyName((aiTextureType)type);
    //            property._type = scene::Material::PropertyType::Texture;
    //            property._data = scene::Material::Property::TextureProperty{ path.C_Str() };
    //            property >> materialStream;
    //        }
    //    }
    //    aiString name = material->GetName();

    //    resource::MaterialHeader materialHeader;
    //    resource::ResourceHeader::fillResourceHeader(&materialHeader, name.C_Str(), materialStream->size(), modelStream->size() + sizeof(scene::MaterialHeader));
    //    materialHeader._numProperties = propertiesCount;
    //    streamMaterialsSize += materialHeader >> modelStream;

    //    materialStream->seekBeg(0);
    //    void* data = materialStream->map(materialStream->size());
    //    modelStream->write(data, materialStream->size());
    //    streamMaterialsSize += materialStream->size();
    //    materialStream->unmap();
    //    stream::StreamManager::destroyStream(materialStream);

    //    LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: Load material[%d] name %s", m, name.C_Str());
    //}

    //LOG_DEBUG("MeshAssimpDecoder::decodeMesh: load materials: %d, size %d bytes", scene->mNumMaterials, streamMaterialsSize);
    //return streamMaterialsSize;
    return 0;
}

u32 AssimpDecoder::decodeAABB(const aiScene* scene, stream::Stream* stream) const
{
    return 0;
}

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP
