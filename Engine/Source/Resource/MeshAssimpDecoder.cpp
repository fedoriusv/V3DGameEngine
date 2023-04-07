#include "MeshAssimpDecoder.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/PipelineStateProperties.h"
#include "Renderer/Formats.h"
#include "Stream/StreamManager.h"
#include "Resource/ModelFileLoader.h"
#include "Scene/Material.h"
#include "Scene/Geometry/Mesh.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#ifdef USE_ASSIMP
#   include <assimp/Importer.hpp>
#   include <assimp/scene.h>
#   include <assimp/postprocess.h>
#   include <assimp/cimport.h>

#define LOG_LOADIMG_TIME 1

namespace v3d
{
namespace resource
{

MeshAssimpDecoder::MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModelHeader& header, ModelFileLoader::ModelLoaderFlags flags) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_headerRules(flags & ModelFileLoader::ReadHeader)

    , m_skipIndices(flags & ModelFileLoader::ModelLoaderFlag::SkipIndexBuffer)
    , m_skipNormals(flags & ModelFileLoader::ModelLoaderFlag::SkipNormalsAttribute)
    , m_skipTangents(flags & ModelFileLoader::ModelLoaderFlag::SkipTangentAndBitangentAttribute)
    , m_skipTextureCoords(flags & ModelFileLoader::ModelLoaderFlag::SkipTextureCoordtAttributes)

    , m_seperatePosition(flags& ModelFileLoader::ModelLoaderFlag::SeperatePositionAttribute)
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
#endif
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

        u32 modelStreamSize = 0;
        stream::Stream* modelStream = stream::StreamManager::createMemoryStream();
        if (m_headerRules)
        {
            if (scene->HasMeshes() && (m_header._modelContentFlags & scene::ModelHeader::ModelContent_Mesh))
            {
                MeshAssimpDecoder::decodeMesh(scene, modelStream, &m_header, 0);
            }

            if (scene->HasMaterials() && (m_header._modelContentFlags & scene::ModelHeader::ModelContent_Material))
            {
                MeshAssimpDecoder::decodeMaterial(scene, modelStream, &m_header);
            }
        }
        else
        {
            scene::ModelHeader newHeader(m_header);
            newHeader._numMeshes = scene->mNumMeshes;
            newHeader._numMaterials = scene->mNumMaterials;
            newHeader._localTransform = m_localTransform;
#if DEBUG
            newHeader._name = name;
#endif

            modelStreamSize += newHeader >> modelStream;

            if (scene->HasMeshes())
            {
                modelStreamSize += MeshAssimpDecoder::decodeMesh(scene, modelStream, &newHeader, activeFlags);
                newHeader._modelContentFlags |= scene::ModelHeader::ModelContent_Mesh;
            }

            if (scene->HasMaterials())
            {
                modelStreamSize += MeshAssimpDecoder::decodeMaterial(scene, modelStream, &newHeader);
                newHeader._modelContentFlags |= scene::ModelHeader::ModelContent_Material;
            }

            //update model header
            modelStream->seekBeg(0);
            newHeader._size = modelStreamSize;
            newHeader._offset = 0;
            newHeader._extraFlags = 0;

            newHeader >> modelStream;
        }

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_INFO("MeshAssimpDecoder::decode: model %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif

        scene::Model* model = new scene::Model();
        model->init(modelStream);

        return model;
    }

    ASSERT(false, "empty");
    return nullptr;
}

u32 MeshAssimpDecoder::decodeMesh(const aiScene* scene, stream::Stream* modelStream, scene::ModelHeader* newHeader, u32 activeFlags) const
{
    u32 streamMeshesSize = 0;
    u32 streamMeshesOffset = 0;
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
                attrib._bindingId = 0;
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
                attrib._bindingId = 0;
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
                    attrib._bindingId = 0;
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
                    attrib._bindingId = 0;
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
                    attrib._bindingId = 0;
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
                    attrib._bindingId = 0;
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
#ifdef DEBUG
        u64 memorySize = 0;
        std::vector<math::Vector3D> vertices;
#endif //DEBUG
        stream::Stream* meshStream = stream::StreamManager::createMemoryStream();

        u32 bindingIndex = 0;
        std::vector<renderer::VertexInputAttributeDescription::InputBinding> inputBindings;
        if (m_seperatePosition)
        {
            u32 stride = buildVertexData(mesh, m_header, scene::MeshHeader::VertexProperies::VertexProperies_Position);
            ASSERT(stride > 0, "invalid stride");
            streamMeshesSize += stride * mesh->mNumVertices;

            inputBindings.push_back(renderer::VertexInputAttributeDescription::InputBinding(bindingIndex, renderer::VertexInputAttributeDescription::InputRate_Vertex, stride));

            if(mesh->HasPositions())
            {
                for (u32 v = 0; v < mesh->mNumVertices; v++)
                {
                    math::Vector3D position;
                    position.m_x = mesh->mVertices[v].x;
                    position.m_y = (m_flipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                    position.m_z = mesh->mVertices[v].z;
                    meshStream->write<math::Vector3D>(position);
#ifdef DEBUG
                    memorySize += sizeof(math::Vector3D);
                    vertices.push_back(position);
#endif //DEBUG
                }
            }

            activeFlags |= ~scene::MeshHeader::VertexProperies::VertexProperies_Position;
            ++bindingIndex;
        }

        u32 stride = buildVertexData(mesh, m_header, activeFlags);
        ASSERT(stride > 0, "invalid stride");
        streamMeshesSize += stride * mesh->mNumVertices;

        inputBindings.push_back(renderer::VertexInputAttributeDescription::InputBinding(bindingIndex, renderer::VertexInputAttributeDescription::InputRate_Vertex, stride));

        for (u32 v = 0; v < mesh->mNumVertices; v++)
        {
            if (mesh->HasPositions() && (activeFlags & scene::MeshHeader::VertexProperies::VertexProperies_Position))
            {
                math::Vector3D position;
                position.m_x = mesh->mVertices[v].x;
                position.m_y = (m_flipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                position.m_z = mesh->mVertices[v].z;
                meshStream->write<math::Vector3D>(position);
#ifdef DEBUG
                memorySize += sizeof(math::Vector3D);

                auto found = std::find(vertices.begin(), vertices.end(), position);
                //ASSERT(found == vertices.end(), "must be unique");
                vertices.push_back(position);
#endif //DEBUG
            }

            if (mesh->HasNormals() && (activeFlags & scene::MeshHeader::VertexProperies::VertexProperies_Normals))
            {
                math::Vector3D normal;
                normal.m_x = mesh->mNormals[v].x;
                normal.m_y = mesh->mNormals[v].y;
                normal.m_z = mesh->mNormals[v].z;
                meshStream->write<math::Vector3D>(normal);
#ifdef DEBUG
                memorySize += sizeof(math::Vector3D);
#endif //DEBUG
            }

            if (mesh->HasTangentsAndBitangents() && 
                (activeFlags & (scene::MeshHeader::VertexProperies::VertexProperies_Tangent | scene::MeshHeader::VertexProperies::VertexProperies_Bitangent)))
            {
                math::Vector3D tangent;
                tangent.m_x = mesh->mTangents[v].x;
                tangent.m_y = mesh->mTangents[v].y;
                tangent.m_z = mesh->mTangents[v].z;
                meshStream->write<math::Vector3D>(tangent);
#ifdef DEBUG
                memorySize += sizeof(math::Vector3D);
#endif //DEBUG

                math::Vector3D bitangent;
                bitangent.m_x = mesh->mBitangents[v].x;
                bitangent.m_y = mesh->mBitangents[v].y;
                bitangent.m_z = mesh->mBitangents[v].z;
                meshStream->write<math::Vector3D>(bitangent);
#ifdef DEBUG
                memorySize += sizeof(math::Vector3D);
#endif //DEBUG
            }

            for (u32 uv = 0; uv < scene::k_maxTextureCoordsCount; uv++)
            {
                if (mesh->HasTextureCoords(uv) && (activeFlags & scene::MeshHeader::VertexProperies::VertexProperies_TextCoord0 + uv))
                {
                    math::Vector2D coord;
                    coord.m_x = mesh->mTextureCoords[uv][v].x;
                    coord.m_y = (m_flipYTexCoord) ? -mesh->mTextureCoords[uv][v].y : mesh->mTextureCoords[uv][v].y;
                    meshStream->write<math::Vector2D>(coord);
#ifdef DEBUG
                    memorySize += sizeof(math::Vector2D);
#endif //DEBUG
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
#ifdef DEBUG
                    memorySize += sizeof(math::Vector4D);
#endif //DEBUG
                }
            }
        }
#ifdef DEBUG
        ASSERT(memorySize == meshStream->size(), "different sizes");
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

            u32 indexSize = static_cast<u32>(index32Buffer.size()) * sizeof(u32);
            streamMeshesSize += indexSize;
#ifdef DEBUG
            memorySize += indexSize;
#endif //DEBUG

            meshStream->write(index32Buffer.data(), indexSize, 1);
        }
#ifdef DEBUG
        ASSERT(memorySize == meshStream->size(), "different sizes");
#endif //DEBUG

        scene::MeshHeader meshHeader;
        meshHeader._size = meshStream->size() + sizeof(scene::MeshHeader);
        meshHeader._offset = streamMeshesOffset;
        meshHeader._version = 0;
        meshHeader._extraFlags = 0;

        meshHeader._numVertices = mesh->mNumVertices;
        meshHeader._vertexStride = stride;
        meshHeader._numIndices = static_cast<u32>(index32Buffer.size());
        meshHeader._indexType = renderer::StreamIndexBufferType::IndexType_32;
        meshHeader._polygonMode = renderer::PolygonMode::PolygonMode_Fill;
        meshHeader._frontFace = renderer::FrontFace::FrontFace_Clockwise;
        meshHeader._vertexContentFlags = contentFlag;
        meshHeader._geometryContentFlags |= m_skipIndices ? 0 :scene::MeshHeader::GeometryContentFlag::IndexBuffer;
        meshHeader._geometryContentFlags |= m_seperatePosition ? scene::MeshHeader::GeometryContentFlag::SeparatePostionAttribute : 0;
        meshHeader._geometryContentFlags |= m_generateBoundingBox ? scene::MeshHeader::GeometryContentFlag::BoundingBox : 0;
#if DEBUG
        meshHeader._name = mesh->mName.C_Str();
#endif
        streamMeshesSize += meshHeader >> modelStream;

        renderer::VertexInputAttributeDescription attribDescriptionList(inputBindings, inputAttributes);
        streamMeshesSize += attribDescriptionList >> modelStream;

        meshStream->seekBeg(0);
        void* data = meshStream->map(meshStream->size());
        modelStream->write(data, meshStream->size());
        meshStream->unmap();
        V3D_DELETE(meshStream, MemoryLabel::MemoryDefault);

        streamMeshesOffset += streamMeshesSize;
    }

    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: load meshes: %d, size %d bytes", scene->mNumMeshes, streamMeshesSize);
    return streamMeshesSize;
}

bool MeshAssimpDecoder::decodeMaterial(const aiScene * scene, stream::Stream * stream, scene::ModelHeader * newHeader) const
{
//    newHeader->_materials.resize(scene->mNumMaterials);
//    for (u32 m = 0; m < scene->mNumMaterials; m++)
//    {
//        aiMaterial* material = scene->mMaterials[m];
//        scene::MaterialHeader& materialHeader = newHeader->_materials[m];
//
//#if DEBUG
//        aiString name;
//        material->Get(AI_MATKEY_NAME, name);
//        materialHeader._debugName = name.C_Str();
//        LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: Load material index %d, name %s", m, name.C_Str());
//#endif
//
//        std::tuple<std::string, aiTextureType, scene::MaterialHeader::Property, bool> vectorProp[] =
//        {
//            { "$clr.diffuse", aiTextureType_DIFFUSE, scene::MaterialHeader::Property_Diffuse, true },
//            { "$clr.ambient", aiTextureType_AMBIENT, scene::MaterialHeader::Property_Ambient, true },
//            { "$clr.specular", aiTextureType_SPECULAR, scene::MaterialHeader::Property_Specular, true },
//            { "$clr.emissive", aiTextureType_EMISSIVE, scene::MaterialHeader::Property_Emission, true },
//
//            { "", aiTextureType_NORMALS, scene::MaterialHeader::Property_Normals, false },
//            { "", aiTextureType_HEIGHT, scene::MaterialHeader::Property_Heightmap, false },
//
//            { "$mat.shininess", aiTextureType_SHININESS, scene::MaterialHeader::Property_Shininess, false },
//            { "$mat.opacity", aiTextureType_OPACITY, scene::MaterialHeader::Property_Opacity, false },
//        };
//
//        for (auto& iter : vectorProp)
//        {
//            scene::MaterialHeader::PropertyInfo info;
//
//            aiReturn result = aiReturn_FAILURE;
//            if (std::get<3>(iter))
//            {
//                aiColor4D color;
//
//                result = material->Get(std::get<0>(iter).c_str(), 0, 0, color);
//                if (result == aiReturn_SUCCESS)
//                {
//                   core::Vector4D value;
//                   value.x = color.r;
//                   value.y = color.g;
//                   value.z = color.b;
//                   value.w = color.a;
//
//                   info._value = value;
//                }
//            }
//            else
//            {
//                f32 value;
//                result = material->Get(std::get<0>(iter).c_str(), 0, 0, value);
//                if (result == aiReturn_SUCCESS)
//                {
//                    info._value = value;
//                }
//            }
//
//            bool texturePresent = material->GetTextureCount(std::get<1>(iter)) > 0;
//            if (texturePresent)
//            {
//                aiString texture;
//                material->GetTexture(std::get<1>(iter), 0, &texture);
//                info._name = texture.C_Str();
//            }
//
//            if (result == aiReturn_SUCCESS || texturePresent)
//            {
//                materialHeader._properties.emplace(std::make_pair(std::get<2>(iter), info));
//            }
//        }
//    }
//
//    LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: load materials: %d", newHeader->_materials.size());
    return true;
}

bool MeshAssimpDecoder::decodeAABB(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header) const
{
    return false;
}

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP
