#include "AssimpDecoder.h"

#include "Renderer/Formats.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#include "Scene/Model.h"
#include "Scene/Geometry/Mesh.h"
#include "Scene/Material.h"
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

AssimpDecoder::AssimpDecoder(renderer::Device* device, const std::vector<std::string>& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_device(device)
{
}

AssimpDecoder::AssimpDecoder(renderer::Device* device, std::vector<std::string>&& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_device(device)
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
            aiProcess_FindDegenerates |
            aiProcess_FindInvalidData |
            aiProcess_LimitBoneWeights |
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

        if (flags & ModelFileLoader::Optimization)
        {
            assimpFlags |= 
                aiProcess_OptimizeMeshes | 
                aiProcess_OptimizeGraph | 
                aiProcess_SplitLargeMeshes |
                aiProcess_RemoveRedundantMaterials |
                0;
        }

        //if (!(flags & ModelFileLoader::LocalTransform))
        //{
        //    assimpFlags |= aiProcess_PreTransformVertices;
        //}

        const u8* data = stream->map(stream->size());
        ASSERT(data, "nullptr");

        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_IMPORT_TER_MAKE_UVS, 1);
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
        importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.f);
        importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, modlePolicy->scaleFactor);

        //FBX
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ALL_GEOMETRY_LAYERS, true);
        //importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ALL_MATERIALS, true);
        //importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, false);

        std::string fileExtension = stream::FileLoader::getFileExtension(name);
        const aiScene* scene = importer.ReadFileFromMemory(data, stream->size(), assimpFlags, fileExtension.c_str());
        stream->unmap();
        if (!scene)
        {
            LOG_ERROR("AssimpDecoder::decode: Load of model [%s] is failed: %s", name.c_str(), importer.GetErrorString());

            ASSERT(false, "nullptr");
            return nullptr;
        }

        u32 modelStreamSize = 0;
        stream::Stream* modelStream = stream::StreamManager::createMemoryStream();

        modelStreamSize += AssimpDecoder::decodeMaterial(scene, modelStream, flags, modlePolicy->overridedShadingModel);
        modelStreamSize += AssimpDecoder::decodeLight(scene, modelStream, flags);
        modelStreamSize += AssimpDecoder::decodeCamera(scene, modelStream, flags);
        modelStreamSize += AssimpDecoder::decodeNode(scene, scene->mRootNode, modelStream, flags, vertexProps);

        scene::Model::ModelHeader header;
        ResourceHeader::fill(&header, name, modelStreamSize, 0);

        Resource* model = V3D_NEW(scene::Model, memory::MemoryLabel::MemoryObject)(m_device, header);
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

u32 AssimpDecoder::decodeNode(const aiScene* scene, const aiNode* node, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags, u32 vertexPropFlags) const
{
    ASSERT(stream, "nullptr");
    u32 streamNodeSize = 0;

    std::string name = node->mName.C_Str();
    stream->write(name);
    streamNodeSize += name.size() + sizeof(u32);

    math::Matrix4D tranform;
    tranform.set(node->mTransformation[0]);
    stream->write<math::Matrix4D>(tranform);
    streamNodeSize += sizeof(math::Matrix4D);

    u32 numChildren = (node->mNumMeshes > 1) ? node->mNumChildren + node->mNumMeshes : node->mNumChildren;
    stream->write<u32>(numChildren);
    streamNodeSize += sizeof(u32);

    //lights
    {
        bool nodeHasLights = scene->HasLights() && !(flags & ModelFileLoader::ModelLoaderFlag::SkipLights);
        u32 lightID = 0;
        if (nodeHasLights)
        {
            nodeHasLights = false;
            for (u32 l = 0; l < scene->mNumLights; ++l)
            {
                if (strcmp(node->mName.C_Str(), scene->mLights[l]->mName.C_Str()) == 0)
                {
                    lightID = l;
                    nodeHasLights = true;
                    break;
                }
            }
        }

        stream->write<bool>(nodeHasLights);
        streamNodeSize += sizeof(bool);

        if (nodeHasLights)
        {
            stream->write<u32>(lightID);
            streamNodeSize += sizeof(u32);
        }
    }

    // meshes
    if (node->mNumMeshes > 0)
    {
        if (node->mNumMeshes == 1)
        {
            u32 index = node->mMeshes[0];
            aiMesh* mesh = scene->mMeshes[index];

            u32 numLODs = 1; //Assimp doesn't support LODs loading, always one
            stream->write<u32>(numLODs);
            streamNodeSize += sizeof(u32);

            streamNodeSize += AssimpDecoder::decodeMesh(scene, mesh, stream, flags, vertexPropFlags);

            //MatrialID
            u32 mateialID = mesh->mMaterialIndex;
            stream->write<u32>(mateialID);
            streamNodeSize += sizeof(u32);
        }
        else //create sub node for each mesh
        {
            u32 numLODs = 0;
            stream->write<u32>(numLODs);
            streamNodeSize += sizeof(u32);

            for (u32 m = 0; m < node->mNumMeshes; ++m)
            {
                u32 index = node->mMeshes[m];
                aiMesh* mesh = scene->mMeshes[index];

                // Sub-node data
                {
                    std::string name = mesh->mName.C_Str();
                    stream->write(name);
                    streamNodeSize += name.size() + sizeof(u32);

                    math::Matrix4D tranform;
                    tranform.makeIdentity();
                    stream->write<math::Matrix4D>(tranform);
                    streamNodeSize += sizeof(math::Matrix4D);

                    u32 numChildren = 0;
                    stream->write<u32>(numChildren);
                    streamNodeSize += sizeof(u32);

                    bool nodeHasLights = false;
                    stream->write<bool>(nodeHasLights);
                    streamNodeSize += sizeof(bool);
                }

                u32 numLODs = 1;
                stream->write<u32>(numLODs);
                streamNodeSize += sizeof(u32);

                streamNodeSize += AssimpDecoder::decodeMesh(scene, mesh, stream, flags, vertexPropFlags);

                //MatrialID
                u32 mateialID = mesh->mMaterialIndex;
                stream->write<u32>(mateialID);
                streamNodeSize += sizeof(u32);
            }
        }
    }
    else
    {
        u32 numLODs = 0;
        stream->write<u32>(numLODs);
        streamNodeSize += sizeof(u32);
    }

    for (u32 n = 0; n < node->mNumChildren; ++n)
    {
        const aiNode* childNode = node->mChildren[n];
        const aiMetadata* metadata = childNode->mMetaData;
        if (metadata)
        {
            for (u32 p = 0; p < metadata->mNumProperties; ++p)
            {
                aiString name = metadata->mKeys[p];
                aiMetadataEntry prop = metadata->mValues[p];

                //TODO
                int i = 0;
            }
        }

        streamNodeSize += AssimpDecoder::decodeNode(scene, childNode, stream, flags, vertexPropFlags);
    }

    return streamNodeSize;
}

u32 AssimpDecoder::decodeMesh(const aiScene* scene, const aiMesh* mesh, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags, u32 vertexPropFlags) const
{
    ASSERT(stream, "nullptr");
    u32 meshStreamSize = 0;
    stream::Stream* meshStream = stream::StreamManager::createMemoryStream();

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

            for (u32 i = enumTypeToIndex(ModelFileLoader::VertexProperies::VertexProperies_TextCoord0); i <= enumTypeToIndex(ModelFileLoader::VertexProperies::VertexProperies_TextCoord3); ++i)
            {
                u32 uv = 1 << i;
                if (presentFlags & uv)
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

            for (u32 i = enumTypeToIndex(ModelFileLoader::VertexProperies::VertexProperies_Color0); i <= enumTypeToIndex(ModelFileLoader::VertexProperies::VertexProperies_Color3); ++i)
            {
                u32 c = 1 << i;
                if (presentFlags & c)
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

    std::string name = mesh->mName.C_Str();
    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Load mesh name %s, material index %d", name.c_str(), mesh->mMaterialIndex);
    ASSERT((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != 0, "must be triangle");

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
    meshStream->write<renderer::PrimitiveTopology>(renderer::PrimitiveTopology_TriangleList);
    meshStreamSize += sizeof(renderer::PrimitiveTopology);

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

        if (mesh->HasPositions())
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

        for (u32 i = enumTypeToIndex(ModelFileLoader::VertexProperies::VertexProperies_TextCoord0), j = 0; i <= enumTypeToIndex(ModelFileLoader::VertexProperies::VertexProperies_TextCoord3); ++i, ++j)
        {
            u32 uv = 1 << i;
            if (vertexPropFlags & uv)
            {
                math::float2 coord;
                if (mesh->HasTextureCoords(j))
                {
                    coord._x = mesh->mTextureCoords[j][v].x;
                    coord._y = (flags & ModelFileLoader::FlipYTextureCoord) ? -mesh->mTextureCoords[j][v].y : mesh->mTextureCoords[j][v].y;
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

        for (u32 i = enumTypeToIndex(ModelFileLoader::VertexProperies::VertexProperies_Color0), j = 0; i <= enumTypeToIndex(ModelFileLoader::VertexProperies::VertexProperies_Color3); ++i, ++j)
        {
            u32 c = 1 << i;
            if (vertexPropFlags & c)
            {
                math::float4 color;
                if (mesh->HasVertexColors(j))
                {
                    color._x = mesh->mColors[j][v].r;
                    color._y = mesh->mColors[j][v].g;
                    color._z = mesh->mColors[j][v].b;
                    color._w = mesh->mColors[j][v].a;
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

    //Indices
    std::vector<u32> index32Buffer;
    if (!(flags & ModelFileLoader::SkipIndexBuffer))
    {
        for (u32 f = 0; f < mesh->mNumFaces; f++)
        {
            const aiFace& face = mesh->mFaces[f];
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

    //AABB
    math::TVector3D<f32> min(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z);
    math::TVector3D<f32> max(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
    math::AABB aabb(min, max);

    meshStream->write<math::AABB>(aabb);
    meshStreamSize += sizeof(math::AABB);


    scene::Mesh::MeshHeader header;
    ResourceHeader::fill(&header, name, meshStreamSize, stream->tell() + sizeof(resource::ResourceHeader));
    meshStreamSize += header >> stream;

    meshStream->seekBeg(0);
    stream->write(meshStream->map(), meshStream->size());
    meshStream->unmap();
    meshStreamSize =+ meshStream->size();
    stream::StreamManager::destroyStream(meshStream);

    return meshStreamSize;
}

u32 AssimpDecoder::decodeSkeleton(const aiScene* scene, stream::Stream* stream) const
{
    return u32();
}

u32 AssimpDecoder::decodeMaterial(const aiScene* scene, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags, scene::MaterialShadingModel overridedShadingModel) const
{
    ASSERT(stream, "nullptr");

    u32 materialsStreamSize = 0;
    u32 mainStreamOffset = stream->tell();
    if (flags & ModelFileLoader::ModelLoaderFlag::SkipMaterial)
    {
        stream->write<u32>(0);
        materialsStreamSize += sizeof(u32);

        return materialsStreamSize;
    }

    stream->write<u32>(scene->mNumMaterials);
    materialsStreamSize += sizeof(u32);


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

    auto parseProperty = [&parseUnknownProperty, &flags, &overridedShadingModel](aiMaterial* material, aiMaterialProperty* materialProperty, u32 id, stream::Stream* stream) -> bool
    {
        if (materialProperty->mKey == aiString("$mat.twosided"))
        {
            ASSERT(materialProperty->mType == aiPTI_Integer, "wrong type");
            s32 value;
            aiReturn result = material->Get(AI_MATKEY_TWOSIDED, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "Twosided";

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
            stream->write<f32>(value);

            return true;
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

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.transparencyfactor"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_TRANSPARENCYFACTOR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "TransparencyFactor";

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.bumpscaling"))
        {
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_BUMPSCALING, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "BumpScaling";

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
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

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
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

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
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

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
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

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
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
            math::float4 color = { value.r,  value.g,  value.b, value.a };

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Vector);
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
            math::float4 color = { value.r,  value.g,  value.b, value.a };

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Vector);
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
            math::float4 color = { value.r,  value.g,  value.b, value.a };

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Vector);
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
            math::float4 color = { value.r,  value.g,  value.b, value.a };

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Vector);
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
            math::float4 color = { value.r,  value.g,  value.b, value.a };

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Vector);
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
            math::float4 color = { value.r,  value.g,  value.b, value.a };

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Vector);
            stream->write<math::float4>(color);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.useColorMap"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_USE_COLOR_MAP, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            return false;
        }
        else if (materialProperty->mKey == aiString("$mat.base"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_BASE_COLOR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            return false;
        }
        else if (materialProperty->mKey == aiString("$mat.metallicFactor"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_METALLIC_FACTOR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "MetallicFactor";

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.roughnessFactor"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_ROUGHNESS_FACTOR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "RoughnessFactor";

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.specularFactor"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_SPECULAR_FACTOR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "SpecularFactor";

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
            stream->write<f32>(value);

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.glossinessFactor"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_GLOSSINESS_FACTOR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            std::string name = "GlossinessFactor";

            stream->write(name);
            stream->write<PropertyType>(PropertyType::Scalar);
            stream->write<f32>(value);

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
        //{ "$mat.shadingm", parseProperty },
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

        //PBR
        { "$mat.useColorMap", parseProperty },

        // Metallic/Roughness Workflow
        { "$clr.base", parseProperty },
        { "$mat.metallicFactor", parseProperty },
        { "$mat.roughnessFactor", parseProperty },

        // Specular/Glossiness Workflow
        { "$mat.specularFactor", parseProperty },
        { "$mat.glossinessFactor", parseProperty },

        { "?bg.global", parseProperty },
        { "?sh.lang", parseProperty },
        { "?sh.vs", parseProperty },
        { "?sh.fs", parseProperty },
        { "?sh.gs", parseProperty },
        { "?sh.ts", parseProperty },
        { "?sh.ps", parseProperty },
        { "?sh.cs", parseProperty },
    };

    stream::Stream* materialsStream = stream::StreamManager::createMemoryStream();
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
                return "Normals";
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
                std::string texture = path.C_Str();
                u8 type = PropertyType::Texture;

                materialStream->write(name);
                materialStream->write<u8>(type);
                materialStream->write(texture);
                ++propertiesCount;
            }
        }

        scene::Material::MaterialHeader header;
        resource::ResourceHeader::fill(&header, name, materialStream->size() + sizeof(propertiesCount) + sizeof(scene::MaterialShadingModel), mainStreamOffset + materialsStreamSize + sizeof(ResourceHeader));

        scene::MaterialShadingModel shadingModel = overridedShadingModel;
        if (!(flags & ModelFileLoader::ModelLoaderFlag::OverridedShadingModel))
        {
            shadingModel = scene::MaterialShadingModel::Custom;
            if (s32 value; material->Get(AI_MATKEY_SHADING_MODEL, value) == aiReturn_SUCCESS)
            {
                aiShadingMode model = (aiShadingMode)value;
                if (model == aiShadingMode_NoShading)
                {
                    shadingModel = scene::MaterialShadingModel::Unlit;
                }
                else if (model == aiShadingMode_OrenNayar || model == aiShadingMode_Minnaert || model == aiShadingMode_Phong || model == aiShadingMode_Blinn)
                {
                    shadingModel = scene::MaterialShadingModel::PBR_Specular;
                }
                else if (model == aiShadingMode_CookTorrance || model == aiShadingMode_PBR_BRDF)
                {
                    shadingModel = scene::MaterialShadingModel::PBR_MetallicRoughness;
                }
            }
        }

        materialsStreamSize += header >> materialsStream;
        materialsStream->write<scene::MaterialShadingModel>(shadingModel);
        materialsStreamSize += sizeof(u32);
        materialsStream->write<u32>(propertiesCount);
        materialsStreamSize += sizeof(u32);

        materialStream->seekBeg(0);
        materialsStream->write(materialStream->map(), materialStream->size());
        materialsStreamSize += materialStream->size();
        materialStream->unmap();
        stream::StreamManager::destroyStream(materialStream);

        LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: Load material[%d] name %s", m, name.c_str());
    }

    ASSERT(materialsStreamSize == materialsStream->size() + sizeof(u32), "must be equal");
    materialsStream->seekBeg(0);
    stream->write(materialsStream->map(), materialsStream->size());
    materialsStream->unmap();
    stream::StreamManager::destroyStream(materialsStream);

    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: load materials: %d, size %d bytes", scene->mNumMaterials, materialsStreamSize);
    return materialsStreamSize;
}

u32 AssimpDecoder::decodeLight(const aiScene* scene, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags) const
{
    ASSERT(stream, "nullptr");

    u32 lightsStreamSize = 0;
    if (flags & ModelFileLoader::ModelLoaderFlag::SkipLights)
    {
        stream->write<u32>(0);
        lightsStreamSize += sizeof(u32);

        return lightsStreamSize;
    }

    stream->write<u32>(scene->mNumLights);
    lightsStreamSize += sizeof(u32);

    u32 mainStreamOffset = stream->tell();
    for (u32 i = 0; i < scene->mNumLights; ++i)
    {
        aiLight* light = scene->mLights[i];
        std::string name = light->mName.C_Str();

        f32 intensity = std::max(light->mColorDiffuse.r, std::max(light->mColorDiffuse.g, light->mColorDiffuse.b));
        f32 temperature = 2500.0f;
        math::float4 attenuation = { light->mAttenuationConstant, light->mAttenuationLinear, light->mAttenuationQuadratic };
        bool shadowCast = true;

        color::ColorRGBAF color;
        color._x = light->mColorDiffuse.r / intensity;
        color._y = light->mColorDiffuse.g / intensity;
        color._z = light->mColorDiffuse.b / intensity;
        color._w = 1.f;

        u32 colorStreamSize =
            sizeof(color) +
            sizeof(intensity) +
            sizeof(temperature) +
            sizeof(attenuation) +
            sizeof(shadowCast);

        scene::Light::Type type;
        if (light->mType == aiLightSourceType::aiLightSource_DIRECTIONAL)
        {
            type = scene::Light::Type::DirectionalLight;

            scene::Light::LightHeader header(type);
            resource::ResourceHeader::fill(&header, name, colorStreamSize, mainStreamOffset + sizeof(ResourceHeader));

            lightsStreamSize += header >> stream;
            lightsStreamSize += stream->write<color::ColorRGBAF>(color);
            lightsStreamSize += stream->write<f32>(intensity);
            lightsStreamSize += stream->write<f32>(temperature);
            lightsStreamSize += stream->write<math::float4>(attenuation);
            lightsStreamSize += stream->write<bool>(shadowCast);
        }
        else if (light->mType == aiLightSourceType::aiLightSource_POINT)
        {
            type = scene::Light::Type::PointLight;

            f32 radius = std::max(light->mSize.x, 1.0f);
            colorStreamSize += sizeof(radius);

            scene::Light::LightHeader header(type);
            resource::ResourceHeader::fill(&header, name, colorStreamSize, mainStreamOffset + sizeof(ResourceHeader));

            lightsStreamSize += header >> stream;
            lightsStreamSize += stream->write<color::ColorRGBAF>(color);
            lightsStreamSize += stream->write<f32>(intensity);
            lightsStreamSize += stream->write<f32>(temperature);
            lightsStreamSize += stream->write<math::float4>(attenuation);
            lightsStreamSize += stream->write<bool>(shadowCast);
            lightsStreamSize += stream->write<f32>(radius);
        }
        else if (light->mType == aiLightSourceType::aiLightSource_SPOT)
        {
            type = scene::Light::Type::SpotLight;
            colorStreamSize += sizeof(f32) + sizeof(f32); //Angles

            scene::Light::LightHeader header(type);
            resource::ResourceHeader::fill(&header, name, colorStreamSize, mainStreamOffset + sizeof(ResourceHeader));

            lightsStreamSize += header >> stream;
            lightsStreamSize += stream->write<color::ColorRGBAF>(color);
            lightsStreamSize += stream->write<f32>(intensity);
            lightsStreamSize += stream->write<f32>(temperature);
            lightsStreamSize += stream->write<math::float4>(attenuation);
            lightsStreamSize += stream->write<bool>(shadowCast);
            lightsStreamSize += stream->write<f32>(light->mAngleInnerCone);
            lightsStreamSize += stream->write<f32>(light->mAngleOuterCone);
        }
        else
        {
            ASSERT(false, "not supported");
        }

        mainStreamOffset += colorStreamSize + sizeof(ResourceHeader);
    }

    return lightsStreamSize;
}

u32 AssimpDecoder::decodeCamera(const aiScene* scene, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags) const
{
    ASSERT(stream, "nullptr");

    u32 camerasStreamSize = 0;
    if (flags & ModelFileLoader::ModelLoaderFlag::SkipCameras)
    {
        stream->write<u32>(0);
        camerasStreamSize += sizeof(u32);

        return camerasStreamSize;
    }

    stream->write<u32>(scene->mNumCameras);
    camerasStreamSize += sizeof(u32);

    u32 mainStreamOffset = stream->tell();
    for (u32 i = 0; i < scene->mNumCameras; ++i)
    {
        aiCamera* camera = scene->mCameras[i];
        std::string name = camera->mName.C_Str();

        f32 clipNear = camera->mClipPlaneNear;
        f32 clipFar = camera->mClipPlaneFar;
        f32 FOV = camera->mHorizontalFOV;
        bool orthographic = camera->mOrthographicWidth > 0.f ? true : false;

        u32 cameraStreamSize =
            sizeof(clipNear) +
            sizeof(clipFar) +
            sizeof(FOV) +
            sizeof(orthographic);

        scene::Camera::CameraHeader header;
        resource::ResourceHeader::fill(&header, name, cameraStreamSize, mainStreamOffset + sizeof(ResourceHeader));

        camerasStreamSize += header >> stream;
        camerasStreamSize += stream->write<f32>(clipNear);
        camerasStreamSize += stream->write<f32>(clipFar);
        camerasStreamSize += stream->write<f32>(FOV);
        camerasStreamSize += stream->write<bool>(orthographic);
        mainStreamOffset += cameraStreamSize + sizeof(ResourceHeader);
    }

    return camerasStreamSize;
}

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP
