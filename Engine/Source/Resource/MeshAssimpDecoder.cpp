#include "MeshAssimpDecoder.h"
#include "Renderer/Formats.h"
#include "Stream/StreamManager.h"
#include "Resource/ModelFileLoader.h"
//#include "Scene/Material.h"
//#include "Scene/StaticMesh.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

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

MeshAssimpDecoder::MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModelHeader& header, ModelFileLoader::ModelLoaderFlags flags) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_headerRules(flags & ModelFileLoader::ReadHeader)

    , m_skipIndices(flags & ModelFileLoader::ModelLoaderFlag::SkipIndexBuffer)
    , m_skipNormals(flags & ModelFileLoader::ModelLoaderFlag::SkipNormals)
    , m_skipTangents(flags & ModelFileLoader::ModelLoaderFlag::SkipTangentAndBitangent)
    , m_skipTextureCoords(flags & ModelFileLoader::ModelLoaderFlag::SkipTextureCoordt)

    , m_seperatePosition(flags& ModelFileLoader::ModelLoaderFlag::SeperatePosition)
    , m_generateBoundingBox(flags& ModelFileLoader::ModelLoaderFlag::UseBoundingBoxes)

    , m_localTransform(flags & ModelFileLoader::ModelLoaderFlag::LocalTransform)
    , m_flipYPosition(flags & ModelFileLoader::ModelLoaderFlag::FlipYPosition)
    , m_flipYTexCoord(flags & ModelFileLoader::ModelLoaderFlag::FlipYTextureCoord)

    , m_splitLargeMeshes(flags & ModelFileLoader::ModelLoaderFlag::SplitLargeMeshes)
    , m_skipMaterialLoading(flags & ModelFileLoader::ModelLoaderFlag::SkipMaterialLoading)
{
}

MeshAssimpDecoder::~MeshAssimpDecoder()
{
}

Resource* MeshAssimpDecoder::decode(const stream::Stream* stream, const std::string& name) const
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

        scene::MeshHeader::VertexProperiesFlags activeFlags = scene::MeshHeader::VertexProperies::VertexProperies_Position;
        if (m_headerRules)
        {
            ASSERT(false, "not implemented");
        }
        else
        {
            assimpFlags = aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_SortByPType;

            if (!m_skipIndices)
            {
                assimpFlags |= aiProcess_JoinIdenticalVertices;
            }

            if (!m_skipNormals)
            {
                assimpFlags |= aiProcess_GenSmoothNormals;
                activeFlags |= scene::MeshHeader::VertexProperies::VertexProperies_Normals;
            }

            if (!m_skipTangents)
            {
                assimpFlags |= aiProcess_CalcTangentSpace;
                activeFlags |= scene::MeshHeader::VertexProperies::VertexProperies_Tangent | scene::MeshHeader::VertexProperies::VertexProperies_Bitangent;
            }

            if (!m_skipTextureCoords)
            {
                assimpFlags |= aiProcess_GenUVCoords;
                activeFlags |= scene::MeshHeader::VertexProperies::VertexProperies_TextCoord0 | scene::MeshHeader::VertexProperies::VertexProperies_TextCoord1 |
                    scene::MeshHeader::VertexProperies::VertexProperies_TextCoord2 | scene::MeshHeader::VertexProperies::VertexProperies_TextCoord3;
            }

            if (m_generateBoundingBox)
            {
                assimpFlags |= aiProcess_GenBoundingBoxes;
            }

            if (m_splitLargeMeshes)
            {
                assimpFlags |= aiProcess_SplitLargeMeshes;
            }

            if (!m_localTransform)
            {
                assimpFlags |= aiProcess_PreTransformVertices;
            }
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

        scene::ModelHeader newHeader(m_header);

        u32 modelStreamSize = 0;
        stream::Stream* modelStream = stream::StreamManager::createMemoryStream();
        if (m_headerRules)
        {
            if (scene->HasMeshes() && (m_header._modelContentFlags & scene::ModelHeader::ModelContent_Mesh))
            {
                MeshAssimpDecoder::decodeMesh(scene, modelStream, &newHeader, 0);
            }

            if (scene->HasMaterials() && (m_header._modelContentFlags & scene::ModelHeader::ModelContent_Material))
            {
                MeshAssimpDecoder::decodeMaterial(scene, modelStream, &newHeader);
            }
        }
        else
        {
            if (scene->HasMeshes())
            {
                modelStreamSize += MeshAssimpDecoder::decodeMesh(scene, modelStream, &newHeader, activeFlags);
                newHeader._modelContentFlags |= scene::ModelHeader::ModelContent_Mesh;
            }

            if (scene->HasMaterials() && !m_skipMaterialLoading)
            {
                modelStreamSize += MeshAssimpDecoder::decodeMaterial(scene, modelStream, &newHeader);
                newHeader._modelContentFlags |= scene::ModelHeader::ModelContent_Material;
            }

            newHeader.fillResourceHeader(&newHeader, name, modelStreamSize, 0);
            newHeader._localTransform = m_localTransform;
        }

        resource::Resource* model = ::V3D_NEW(scene::Model, memory::MemoryLabel::MemoryObject)(V3D_NEW(scene::ModelHeader, memory::MemoryLabel::MemoryObject)(newHeader));
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

u32 MeshAssimpDecoder::decodeMesh(const aiScene* scene, stream::Stream* modelStream, scene::ModelHeader* newHeader, u32 activeFlags) const
{
    ASSERT(modelStream, "nullptr");
    modelStream->write<u32>(scene->mNumMeshes);

    u32 streamMeshesSize = sizeof(u32);
    for (u32 m = 0; m < scene->mNumMeshes; m++)
    {
        scene::MeshHeader::VertexProperiesFlags contentFlag = 0;
        std::vector<renderer::VertexInputAttributeDescription::InputAttribute> inputAttributes;
        static auto buildVertexData = [&inputAttributes, &contentFlag](const aiMesh* mesh, const scene::ModelHeader& header, scene::MeshHeader::VertexProperiesFlags presentFlags) -> u32
        {
            u32 vertexSize = 0;
            if (mesh->HasPositions() && (presentFlags & scene::MeshHeader::VertexProperies::VertexProperies_Position))
            {
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Position vec3");

                renderer::VertexInputAttributeDescription::InputAttribute attrib;
                attrib._bindingID = 0;
                attrib._streamId = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offest = vertexSize;
                inputAttributes.push_back(attrib);

                contentFlag |= scene::MeshHeader::VertexProperies::VertexProperies_Position;
                vertexSize += sizeof(math::Vector3D);
            }

            if (mesh->HasNormals() && (presentFlags & scene::MeshHeader::VertexProperies::VertexProperies_Normals))
            {
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Normals vec3");

                renderer::VertexInputAttributeDescription::InputAttribute attrib;
                attrib._bindingID = 0;
                attrib._streamId = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offest = vertexSize;
                inputAttributes.push_back(attrib);

                contentFlag |= scene::MeshHeader::VertexProperies::VertexProperies_Normals;
                vertexSize += sizeof(math::Vector3D);
            }

            if (mesh->HasTangentsAndBitangents() && 
                (presentFlags & (scene::MeshHeader::VertexProperies::VertexProperies_Tangent | scene::MeshHeader::VertexProperies::VertexProperies_Bitangent)))
            {
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Tangent vec3");

                    renderer::VertexInputAttributeDescription::InputAttribute attrib;
                    attrib._bindingID = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                    attrib._offest = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= scene::MeshHeader::VertexProperies::VertexProperies_Tangent;
                    vertexSize += sizeof(math::Vector3D);
                }
                
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Bitangent vec3");

                    renderer::VertexInputAttributeDescription::InputAttribute attrib;
                    attrib._bindingID = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                    attrib._offest = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= scene::MeshHeader::VertexProperies::VertexProperies_Bitangent;
                    vertexSize += sizeof(math::Vector3D);
                }
            }

            for (u32 uv = 0; uv < scene::k_maxTextureCoordsCount; uv++)
            {
                if (mesh->HasTextureCoords(uv) && (presentFlags & scene::MeshHeader::VertexProperies::VertexProperies_TextCoord0 + uv))
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_TextCoord[%d] vec2", uv);

                    renderer::VertexInputAttributeDescription::InputAttribute attrib;
                    attrib._bindingID = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32_SFloat;
                    attrib._offest = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= scene::MeshHeader::VertexProperies::VertexProperies_TextCoord0 + uv;
                    vertexSize += sizeof(math::Vector2D);
                }
            }

            for (u32 c = 0; c < scene::k_maxVertexColorCount; c++)
            {
                if (mesh->HasVertexColors(c) && (presentFlags & scene::MeshHeader::VertexProperies::VertexProperies_Color0 + c))
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Color[%d] vec2", c);

                    renderer::VertexInputAttributeDescription::InputAttribute attrib;
                    attrib._bindingID = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32B32A32_SFloat;
                    attrib._offest = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= scene::MeshHeader::VertexProperies::VertexProperies_Color0 + c;
                    vertexSize += sizeof(math::Vector4D);
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


        //Calculate size of a vertex
        u32 bindingIndex = 0;
        std::vector<renderer::VertexInputAttributeDescription::InputBinding> inputBindings;
        if (m_seperatePosition)
        {
            u32 stride = buildVertexData(mesh, m_header, scene::MeshHeader::VertexProperies::VertexProperies_Position);
            ASSERT(stride > 0, "invalid stride");
            u32 meshBufferSize = stride * mesh->mNumVertices;

            inputBindings.push_back(renderer::VertexInputAttributeDescription::InputBinding(bindingIndex, renderer::VertexInputAttributeDescription::InputRate::InputRate_Vertex, stride));

            activeFlags |= ~scene::MeshHeader::VertexProperies::VertexProperies_Position;
            ++bindingIndex;
        }
        u32 stride = buildVertexData(mesh, m_header, activeFlags);
        ASSERT(stride > 0, "invalid stride");
        u32 meshBufferSize = stride * mesh->mNumVertices;

        inputBindings.push_back(renderer::VertexInputAttributeDescription::InputBinding(bindingIndex, renderer::VertexInputAttributeDescription::InputRate::InputRate_Vertex, stride));

        renderer::VertexInputAttributeDescription attribDescriptionList(inputBindings, inputAttributes);
        meshStreamSize += attribDescriptionList >> meshStream;


        //Fill data of a vertex
        if (m_seperatePosition)
        {
            u32 stride = buildVertexData(mesh, m_header, scene::MeshHeader::VertexProperies::VertexProperies_Position);
            ASSERT(stride > 0, "invalid stride");
            u32 meshBufferSize = stride * mesh->mNumVertices;

            meshStream->write<u32>(meshBufferSize);
            meshStreamSize += sizeof(u32);

            if(mesh->HasPositions())
            {
                for (u32 v = 0; v < mesh->mNumVertices; v++)
                {
                    math::Vector3D position;
                    position.m_x = mesh->mVertices[v].x;
                    position.m_y = (m_flipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                    position.m_z = mesh->mVertices[v].z;

                    meshStream->write<math::Vector3D>(position);
                    meshStreamSize += sizeof(math::Vector3D);
                }
            }
        }


        meshStream->write<u32>(meshBufferSize);
        meshStreamSize += sizeof(u32);

        for (u32 v = 0; v < mesh->mNumVertices; v++)
        {
            if (mesh->HasPositions() && (activeFlags & scene::MeshHeader::VertexProperies::VertexProperies_Position))
            {
                math::Vector3D position;
                position.m_x = mesh->mVertices[v].x;
                position.m_y = (m_flipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                position.m_z = mesh->mVertices[v].z;

                meshStream->write<math::Vector3D>(position);
                meshStreamSize += sizeof(math::Vector3D);
            }

            if (mesh->HasNormals() && (activeFlags & scene::MeshHeader::VertexProperies::VertexProperies_Normals))
            {
                math::Vector3D normal;
                normal.m_x = mesh->mNormals[v].x;
                normal.m_y = mesh->mNormals[v].y;
                normal.m_z = mesh->mNormals[v].z;

                meshStream->write<math::Vector3D>(normal);
                meshStreamSize += sizeof(math::Vector3D);
            }

            if (mesh->HasTangentsAndBitangents() && 
                (activeFlags & (scene::MeshHeader::VertexProperies::VertexProperies_Tangent | scene::MeshHeader::VertexProperies::VertexProperies_Bitangent)))
            {
                math::Vector3D tangent;
                tangent.m_x = mesh->mTangents[v].x;
                tangent.m_y = mesh->mTangents[v].y;
                tangent.m_z = mesh->mTangents[v].z;

                meshStream->write<math::Vector3D>(tangent);
                meshStreamSize += sizeof(math::Vector3D);

                math::Vector3D bitangent;
                bitangent.m_x = mesh->mBitangents[v].x;
                bitangent.m_y = mesh->mBitangents[v].y;
                bitangent.m_z = mesh->mBitangents[v].z;

                meshStream->write<math::Vector3D>(bitangent);
                meshStreamSize += sizeof(math::Vector3D);
            }

            for (u32 uv = 0; uv < scene::k_maxTextureCoordsCount; uv++)
            {
                if (mesh->HasTextureCoords(uv) && (activeFlags & scene::MeshHeader::VertexProperies::VertexProperies_TextCoord0 + uv))
                {
                    math::Vector2D coord;
                    coord.m_x = mesh->mTextureCoords[uv][v].x;
                    coord.m_y = (m_flipYTexCoord) ? -mesh->mTextureCoords[uv][v].y : mesh->mTextureCoords[uv][v].y;

                    meshStream->write<math::Vector2D>(coord);
                    meshStreamSize += sizeof(math::Vector2D);
                }
            }

            for (u32 c = 0; c < scene::k_maxVertexColorCount; c++)
            {
                if (mesh->HasVertexColors(c) && (activeFlags & scene::MeshHeader::VertexProperies::VertexProperies_Color0 + c))
                {
                    math::Vector4D color;
                    color.m_x = mesh->mColors[c][v].r;
                    color.m_y = mesh->mColors[c][v].g;
                    color.m_z = mesh->mColors[c][v].b;
                    color.m_w = mesh->mColors[c][v].a;

                    meshStream->write<math::Vector4D>(color);
                    meshStreamSize += sizeof(math::Vector4D);
                }
            }
        }
#ifdef DEBUG
        ASSERT(meshStreamSize == meshStream->size(), "different sizes");
#endif //DEBUG

        std::vector<u32> index32Buffer;
        if (!m_skipIndices)
        {
            for (u32 f = 0; f < scene->mMeshes[m]->mNumFaces; f++)
            {
                const aiFace& face = scene->mMeshes[m]->mFaces[f];
                for (u32 i = 0; i < face.mNumIndices; i++)
                {
                    index32Buffer.push_back(face.mIndices[i]);
                }
            }

            u32 indexBufferSize = static_cast<u32>(index32Buffer.size()) * sizeof(u32);

            meshStream->write<u32>(indexBufferSize);
            meshStream->write(index32Buffer.data(), indexBufferSize, 1);
            meshStreamSize += indexBufferSize + sizeof(u32);
        }
#ifdef DEBUG
        ASSERT(meshStreamSize == meshStream->size(), "different sizes");
#endif //DEBUG

        if (m_generateBoundingBox)
        {
            math::Vector3D min(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z);
            math::Vector3D max(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
            math::AABB aabb(min, max);

            meshStream->write<math::AABB>(aabb);
            meshStreamSize += sizeof(math::AABB);
        }
#ifdef DEBUG
        ASSERT(meshStreamSize == meshStream->size(), "different sizes");
#endif //DEBUG

        scene::MeshHeader meshHeader;
        resource::ResourceHeader::fillResourceHeader(&meshHeader, mesh->mName.C_Str(), meshStream->size(), modelStream->size() + sizeof(scene::MeshHeader));
        meshHeader._numVertices = mesh->mNumVertices;
        meshHeader._vertexStride = stride;
        meshHeader._numIndices = static_cast<u32>(index32Buffer.size());
        meshHeader._indexType = renderer::StreamIndexBufferType::IndexType_32;
        meshHeader._polygonMode = renderer::PolygonMode::PolygonMode_Fill;
        meshHeader._frontFace = renderer::FrontFace::FrontFace_Clockwise;
        meshHeader._vertexContentFlags = contentFlag;
        meshHeader._geometryContentFlags |= m_skipIndices ? 0 :scene::MeshHeader::GeometryContentFlag::IndexBuffer;
        meshHeader._geometryContentFlags |= m_seperatePosition ? scene::MeshHeader::GeometryContentFlag::SeparatePostion : 0;
        meshHeader._geometryContentFlags |= m_generateBoundingBox ? scene::MeshHeader::GeometryContentFlag::BoundingBox : 0;
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

u32 MeshAssimpDecoder::decodeMaterial(const aiScene* scene, stream::Stream* modelStream, scene::ModelHeader* newHeader) const
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

    auto parseProperty = [&parseUnknownProperty](aiMaterial* material, aiMaterialProperty* materialProperty, u32 id, stream::Stream* stream) -> bool
    {
        scene::Material::Property property;
        property._index = id;
        property._array = 1;

        if (materialProperty->mKey == aiString("$mat.twosided"))
        {
            ASSERT(materialProperty->mType == aiPTI_Integer, "wrong type");
            s32 value;
            aiReturn result = material->Get(AI_MATKEY_TWOSIDED, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            bool twoSided = value;

            return false;
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
            else if (materialProperty->mType == aiPTI_Buffer)
            {
                if (materialProperty->mDataLength == sizeof(u32))
                {
                    u32 value;
                    aiReturn result = material->Get(AI_MATKEY_SHADING_MODEL, value);
                    ASSERT(result == aiReturn_SUCCESS, "can't read");

                    aiShadingMode model = (aiShadingMode)value;

                    return false;
                }
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

            //TODO

            return false;
        }
        else if (materialProperty->mKey == aiString("$mat.opacity"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_OPACITY, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            property._label = scene::Material::Property_Opacity;
            property._type = scene::Material::PropertyType::Value;
            property._data = scene::Material::Property::ValueProperty{ value };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.transparencyfactor"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_TRANSPARENCYFACTOR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            property._label = scene::Material::Property_Transparent;
            property._type = scene::Material::PropertyType::Value;
            property._data = scene::Material::Property::ValueProperty{ value };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.bumpscaling"))
        {
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_BUMPSCALING, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            property._label = scene::Material::Property_Bump;
            property._type = scene::Material::PropertyType::Value;
            property._data = scene::Material::Property::ValueProperty{ value };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.shininess"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_SHININESS, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            property._label = scene::Material::Property_Shininess;
            property._type = scene::Material::PropertyType::Value;
            property._data = scene::Material::Property::ValueProperty{ value };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.reflectivity"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_REFLECTIVITY, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            property._label = scene::Material::Property_Reflection;
            property._type = scene::Material::PropertyType::Value;
            property._data = scene::Material::Property::ValueProperty{ value };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.shinpercent"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_SHININESS_STRENGTH, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            property._label = scene::Material::Property_Shininess;
            property._type = scene::Material::PropertyType::Value;
            property._data = scene::Material::Property::ValueProperty{ value };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$mat.refracti"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            f32 value;
            aiReturn result = material->Get(AI_MATKEY_REFRACTI, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            property._label = scene::Material::Property_Refract;
            property._type = scene::Material::PropertyType::Value;
            property._data = scene::Material::Property::ValueProperty{ value };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.diffuse"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_DIFFUSE, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            math::Vector4D color;
            color.m_x = value.r;
            color.m_y = value.g;
            color.m_z = value.b;
            color.m_w = value.a;

            property._label = scene::Material::Property_Diffuse;
            property._type = scene::Material::PropertyType::Vector;
            property._data = scene::Material::Property::VectorProperty{ color };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.ambient"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_AMBIENT, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            math::Vector4D color;
            color.m_x = value.r;
            color.m_y = value.g;
            color.m_z = value.b;
            color.m_w = value.a;

            property._label = scene::Material::Property_Ambient;
            property._type = scene::Material::PropertyType::Vector;
            property._data = scene::Material::Property::VectorProperty{ color };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.specular"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_SPECULAR, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            math::Vector4D color;
            color.m_x = value.r;
            color.m_y = value.g;
            color.m_z = value.b;
            color.m_w = value.a;

            property._label = scene::Material::Property_Specular;
            property._type = scene::Material::PropertyType::Vector;
            property._data = scene::Material::Property::VectorProperty{ color };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.emissive"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_EMISSIVE, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            math::Vector4D color;
            color.m_x = value.r;
            color.m_y = value.g;
            color.m_z = value.b;
            color.m_w = value.a;

            property._label = scene::Material::Property_Emission;
            property._type = scene::Material::PropertyType::Vector;
            property._data = scene::Material::Property::VectorProperty{ color };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.transparent"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_TRANSPARENT, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            math::Vector4D color;
            color.m_x = value.r;
            color.m_y = value.g;
            color.m_z = value.b;
            color.m_w = value.a;

            property._label = scene::Material::Property_Transparent;
            property._type = scene::Material::PropertyType::Vector;
            property._data = scene::Material::Property::VectorProperty{ color };
            property >> stream;

            return true;
        }
        else if (materialProperty->mKey == aiString("$clr.reflective"))
        {
            ASSERT(materialProperty->mType == aiPTI_Float, "wrong type");
            aiColor4D value;
            aiReturn result = material->Get(AI_MATKEY_COLOR_REFLECTIVE, value);
            ASSERT(result == aiReturn_SUCCESS, "can't read");

            math::Vector4D color;
            color.m_x = value.r;
            color.m_y = value.g;
            color.m_z = value.b;
            color.m_w = value.a;

            property._label = scene::Material::Property_Reflection;
            property._type = scene::Material::PropertyType::Vector;
            property._data = scene::Material::Property::VectorProperty{ color };
            property >> stream;

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

    for (u32 m = 0; m < scene->mNumMaterials; m++)
    {
        stream::Stream* materialStream = stream::StreamManager::createMemoryStream();
        u32 propertiesCount = 0;

        aiMaterial* material = scene->mMaterials[m];

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

        auto texturePropertyName = [](aiTextureType type) -> scene::Material::PropertyName
        {
            switch (type)
            {
            case aiTextureType::aiTextureType_DIFFUSE:
                return scene::Material::PropertyName::Property_Diffuse;
            case aiTextureType::aiTextureType_SPECULAR:
                return scene::Material::PropertyName::Property_Specular;
            case aiTextureType::aiTextureType_AMBIENT:
                return scene::Material::PropertyName::Property_Ambient;
            case aiTextureType::aiTextureType_EMISSIVE:
                return scene::Material::PropertyName::Property_Emission;
            case aiTextureType::aiTextureType_HEIGHT:
                return scene::Material::PropertyName::Property_Height;
            case aiTextureType::aiTextureType_NORMALS:
                return scene::Material::PropertyName::Property_Normal;
            case aiTextureType::aiTextureType_SHININESS:
                return scene::Material::PropertyName::Property_Shininess;
            case aiTextureType::aiTextureType_OPACITY:
                return scene::Material::PropertyName::Property_Opacity;
            case aiTextureType::aiTextureType_DISPLACEMENT:
                return scene::Material::PropertyName::Property_Displacement;
            case aiTextureType::aiTextureType_LIGHTMAP:
                return scene::Material::PropertyName::Property_Light;
            case aiTextureType::aiTextureType_REFLECTION:
                return scene::Material::PropertyName::Property_Reflection;
            case aiTextureType::aiTextureType_BASE_COLOR:
                return scene::Material::PropertyName::Property_Albedo;
            case aiTextureType::aiTextureType_NORMAL_CAMERA:
                return scene::Material::PropertyName::Property_Normal;
            case aiTextureType::aiTextureType_EMISSION_COLOR:
                return scene::Material::PropertyName::Property_Emission;
            case aiTextureType::aiTextureType_METALNESS:
                return scene::Material::PropertyName::Property_Metallic;
            case aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS:
                return scene::Material::PropertyName::Property_Roughness;
            case aiTextureType::aiTextureType_AMBIENT_OCCLUSION:
                return scene::Material::PropertyName::Property_AmbientOcclusion;

            case aiTextureType::aiTextureType_NONE:
            default:
                ASSERT(false, "not found");
                return scene::Material::PropertyName::Property_Unknown;
            }

            return scene::Material::PropertyName::Property_Unknown;
        };

        for (u32 type = aiTextureType::aiTextureType_NONE; type <= aiTextureType::aiTextureType_UNKNOWN; ++type)
        {
            u32 countTextures = material->GetTextureCount((aiTextureType)type);
            for (u32 t = 0; t < countTextures; ++t)
            {
                u32 textureID = material->mNumProperties + type + t;

                aiString path;
                aiReturn result = material->GetTexture((aiTextureType)type, t, &path);
                ASSERT(result == aiReturn_SUCCESS, "can't read");

                scene::Material::Property property;
                property._index = textureID;
                property._array = 1;
                property._label = texturePropertyName((aiTextureType)type);
                property._type = scene::Material::PropertyType::Texture;
                property._data = scene::Material::Property::TextureProperty{ path.C_Str() };
                property >> materialStream;
            }
        }
        aiString name = material->GetName();

        scene::MaterialHeader materialHeader;
        resource::ResourceHeader::fillResourceHeader(&materialHeader, name.C_Str(), materialStream->size(), modelStream->size() + sizeof(scene::MaterialHeader));
        materialHeader._numProperties = propertiesCount;
        streamMaterialsSize += materialHeader >> modelStream;

        materialStream->seekBeg(0);
        void* data = materialStream->map(materialStream->size());
        modelStream->write(data, materialStream->size());
        streamMaterialsSize += materialStream->size();
        materialStream->unmap();
        stream::StreamManager::destroyStream(materialStream);

        LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: Load material[%d] name %s", m, name.C_Str());
    }

    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: load materials: %d, size %d bytes", scene->mNumMaterials, streamMaterialsSize);
    return streamMaterialsSize;
}

u32 MeshAssimpDecoder::decodeAABB(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header) const
{
    return 0;
}

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP
