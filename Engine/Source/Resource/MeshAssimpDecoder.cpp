#include "MeshAssimpDecoder.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/PipelineStateProperties.h"
#include "Renderer/Formats.h"
#include "Stream/StreamManager.h"
#include "Resource/ModelFileLoader.h"
#include "Scene/Material.h"

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

MeshAssimpDecoder::MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModelHeader& header, u32 flags) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_headerRules(flags & ModelLoaderFlag::ModelLoaderFlag_ReadHeader)
    , m_seperateMesh(flags & ModelLoaderFlag::ModelLoaderFlag_SeperateMesh)
    , m_splitLargeMeshes(flags & ModelLoaderFlag::ModelLoaderFlag_SplitLargeMeshes)
    , m_generateIndices(!(flags & ModelLoaderFlag::ModelLoaderFlag_NoGenerateIndex))
    //, m_generateTextureCoords(flags & ModelLoaderFlag::ModelLoaderFlag_GenerateTextureCoord)
    , m_generateNormals(flags & ModelLoaderFlag::ModelLoaderFlag_GenerateNormals)
    , m_generateTangents(flags & ModelLoaderFlag::ModelLoaderFlag_GenerateTangentAndBitangent)
    , m_useBitangents(flags & ModelLoaderFlag_UseBitangent)
    , m_localTransform(flags & ModelLoaderFlag::ModelLoaderFlag_LocalTransform)
    , m_flipYPosition(flags & ModelLoaderFlag::ModelLoaderFlag_FlipYPosition)
    //TODO add ModelLoaderFlag_SkipNormals
{
}

MeshAssimpDecoder::~MeshAssimpDecoder()
{
}

Resource * MeshAssimpDecoder::decode(const stream::Stream* stream, const std::string& name)
{
    if (stream->size() > 0)
    {
        stream->seekBeg(0);

#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif
        const aiScene* scene;
        Assimp::Importer Importer;

        scene::ModelHeader* newHeader = new scene::ModelHeader(m_header);
        u32 assimpFlags = 0;
        if (m_headerRules)
        {
            ASSERT(false, "not implemented");
        }
        else
        {
            assimpFlags = aiProcess_ConvertToLeftHanded | aiProcess_FlipWindingOrder | aiProcess_Triangulate;

            if (!m_localTransform)
            {
                assimpFlags |= aiProcess_PreTransformVertices;
            }

            /*if (m_generateTextureCoords)
            {
                assimpFlags |= aiProcess_GenUVCoords;
            }*/

            if (m_generateNormals)
            {
                assimpFlags |= aiProcess_GenSmoothNormals;
            }

            if (m_generateTangents)
            {
                assimpFlags |= aiProcess_CalcTangentSpace;
            }

            if (m_splitLargeMeshes)
            {
                assimpFlags |= aiProcess_SplitLargeMeshes;
            }

            if (m_generateIndices && m_seperateMesh)
            {
                assimpFlags |= aiProcess_JoinIdenticalVertices;
            }

            newHeader->_vertexContentFlags = 0;
            newHeader->_mode = renderer::PolygonMode::PolygonMode_Triangle;
            newHeader->_frontFace = renderer::FrontFace::FrontFace_Clockwise;
            newHeader->_localTransform = false;
        }

        u8* data = stream->map(stream->size());
        scene = Importer.ReadFileFromMemory(data, stream->size(), assimpFlags);
        stream->unmap();
        if (!scene)
        {
            ASSERT(false, "nullptr");
            delete newHeader;

            return nullptr;
        }

        stream::Stream* modelStream = stream::StreamManager::createMemoryStream();
        if (scene->HasMeshes())
        {
            if (m_headerRules && (newHeader->_modelContentFlags & scene::ModelHeader::ModelContext_Mesh))
            {
                MeshAssimpDecoder::decodeMesh(scene, modelStream, newHeader);
            }
            else
            {
                MeshAssimpDecoder::decodeMesh(scene, modelStream, newHeader);
                newHeader->_modelContentFlags |= scene::ModelHeader::ModelContext_Mesh;
            }
        }

        if (scene->HasMaterials())
        {
            if (m_headerRules && (newHeader->_modelContentFlags & scene::ModelHeader::ModelContext_Material))
            {
                MeshAssimpDecoder::decodeMaterial(scene, modelStream, newHeader);
            }
            else
            {
                MeshAssimpDecoder::decodeMaterial(scene, modelStream, newHeader);
                newHeader->_modelContentFlags |= scene::ModelHeader::ModelContext_Material;
            }
        }

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_INFO("MeshAssimpDecoder::decode , model %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif

#if DEBUG
        newHeader->_debugName = name;
#endif

        scene::Model* model = new scene::Model(newHeader);
        model->init(modelStream);

        return model;
    }

    ASSERT(false, "empty");
    return nullptr;
}


bool MeshAssimpDecoder::decodeMesh(const aiScene* scene, stream::Stream* modelStream, scene::ModelHeader* newHeader)
{
    u64 globalVertexSize = 0;
    u64 globalIndexSize = 0;
    std::vector<u32> indexBuffer;

    newHeader->_meshes.resize(m_seperateMesh ? scene->mNumMeshes : 1);
    if (!m_seperateMesh)
    {
        newHeader->_meshes[0]._vertex._subData.reserve(scene->mNumMeshes);
    }
    
    std::vector<renderer::VertexInputAttribDescription> attribDescriptionList;

    for (u32 m = 0; m < scene->mNumMeshes; m++)
    {
        std::vector<renderer::VertexInputAttribDescription::InputBinding> inputBindings;
        std::vector<renderer::VertexInputAttribDescription::InputAttribute> inputAttributes;

        scene::ModelHeader::VertexProperiesFlags contentFlag = 0;
        auto buildVertexData = [&inputAttributes, &contentFlag](const aiMesh* mesh, const scene::ModelHeader& header, bool useBitangent) -> u32
        {
            u32 vertexSize = 0;
            if (mesh->HasPositions())
            {
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Position vec3");

                renderer::VertexInputAttribDescription::InputAttribute attrib;
                attrib._bindingId = 0;
                attrib._streamId = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offest = vertexSize;
                inputAttributes.push_back(attrib);

                contentFlag |= scene::ModelHeader::VertexProperies_Position;
                vertexSize += sizeof(core::Vector3D);
            }

            if (mesh->HasNormals())
            {
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Normals vec3");

                renderer::VertexInputAttribDescription::InputAttribute attrib;
                attrib._bindingId = 0;
                attrib._streamId = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offest = vertexSize;
                inputAttributes.push_back(attrib);

                contentFlag |= scene::ModelHeader::VertexProperies_Normals;
                vertexSize += sizeof(core::Vector3D);
            }

            if (mesh->HasTangentsAndBitangents())
            {
                LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Tangent vec3");

                renderer::VertexInputAttribDescription::InputAttribute attrib;
                attrib._bindingId = 0;
                attrib._streamId = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offest = vertexSize;
                inputAttributes.push_back(attrib);

                contentFlag |= scene::ModelHeader::VertexProperies_Tangent;
                vertexSize += sizeof(core::Vector3D);

                if (useBitangent)
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Bitangent vec3");

                    renderer::VertexInputAttribDescription::InputAttribute attrib;
                    attrib._bindingId = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                    attrib._offest = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= scene::ModelHeader::VertexProperies_Bitangent;
                    vertexSize += sizeof(core::Vector3D);
                }
            }

            for (u32 uv = 0; uv < scene::k_maxTextureCoordsIndex; uv++)
            {
                if (mesh->HasTextureCoords(uv))
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_TextCoord[%d] vec2", uv);

                    renderer::VertexInputAttribDescription::InputAttribute attrib;
                    attrib._bindingId = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32_SFloat;
                    attrib._offest = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= scene::ModelHeader::VertexProperies_TextCoord0 + uv;
                    vertexSize += sizeof(core::Vector2D);
                }
            }

            for (u32 c = 0; c < scene::k_maxVertexColorIndex; c++)
            {
                if (mesh->HasVertexColors(c))
                {
                    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Add Attribute VertexProperies_Color[%d] vec2", c);

                    renderer::VertexInputAttribDescription::InputAttribute attrib;
                    attrib._bindingId = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32B32A32_SFloat;
                    attrib._offest = vertexSize;
                    inputAttributes.push_back(attrib);

                    contentFlag |= scene::ModelHeader::VertexProperies_Color0 + c;
                    vertexSize += sizeof(core::Vector4D);
                }
            }

            return vertexSize;
        };

        const aiMesh* mesh = scene->mMeshes[m];
        LOG_DEBUG("MeshAssimpDecoder::decodeMesh: Load mesh index %d, name %s, material index %d", m, mesh->mName.C_Str(), mesh->mMaterialIndex);
        u32 stride = buildVertexData(mesh, m_header, m_useBitangents);
        ASSERT(stride > 0, "invalid stride");
        u64 meshSize = stride * mesh->mNumVertices;

        inputBindings.push_back(renderer::VertexInputAttribDescription::InputBinding(0, renderer::VertexInputAttribDescription::InputRate_Vertex, stride));
        attribDescriptionList.emplace_back(inputBindings, inputAttributes);

        if (m_seperateMesh)
        {
            attribDescriptionList[m] >> modelStream;

            newHeader->_meshes[m]._size = meshSize;
            newHeader->_meshes[m]._offset = globalVertexSize;
            newHeader->_meshes[m]._indexPresent = m_generateIndices;
#if DEBUG
            newHeader->_meshes[m]._debugName = mesh->mName.C_Str();
#endif
            scene::MeshHeader::GeometryInfo& vertexInfo = newHeader->_meshes[m]._vertex;
            vertexInfo._subData.push_back({ globalVertexSize, meshSize, mesh->mNumVertices });
            vertexInfo._count = mesh->mNumVertices;
            vertexInfo._size = meshSize;
        }
        else
        {
            if (m == 0)
            {
                attribDescriptionList.front() >> modelStream;

                newHeader->_meshes.front()._offset = 0;
                newHeader->_meshes.front()._indexPresent = m_generateIndices;
#if DEBUG
                newHeader->_meshes.front()._debugName = scene->mMeshes[0]->mName.C_Str();
#endif
            }
            ASSERT(attribDescriptionList.front() == attribDescriptionList[m], "different descriptions");

            newHeader->_meshes.front()._size += meshSize;

            scene::MeshHeader::GeometryInfo& vertexInfo = newHeader->_meshes.front()._vertex;
            vertexInfo._subData.push_back({ globalVertexSize, meshSize, mesh->mNumVertices });
            vertexInfo._count += mesh->mNumVertices;
            vertexInfo._size += meshSize;
        }

        newHeader->_vertexContentFlags = contentFlag;
        globalVertexSize += meshSize;

#ifdef DEBUG
        u64 memorySize = 0;
#endif //DEBUG
        stream::Stream* meshStream = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(meshSize));
        for (u32 v = 0; v < mesh->mNumVertices; v++)
        {
            if (mesh->HasPositions())
            {
                core::Vector3D position;
                position.x = mesh->mVertices[v].x;
                position.y = (m_flipYPosition) ? -mesh->mVertices[v].y : mesh->mVertices[v].y;
                position.z = mesh->mVertices[v].z;
                meshStream->write<core::Vector3D>(position);
#ifdef DEBUG
                memorySize += sizeof(core::Vector3D);
#endif //DEBUG
            }
            else
            {
                ASSERT(!(m_headerRules && (m_header._vertexContentFlags & scene::ModelHeader::VertexProperies_Position)), "should contain vertex data");
            }

            if (mesh->HasNormals())
            {
                core::Vector3D normal;
                normal.x = mesh->mNormals[v].x;
                normal.y = mesh->mNormals[v].y;
                normal.z = mesh->mNormals[v].z;
                meshStream->write<core::Vector3D>(normal);
#ifdef DEBUG
                memorySize += sizeof(core::Vector3D);
#endif //DEBUG
            }
            else
            {
                ASSERT(!(m_headerRules && (m_header._vertexContentFlags & scene::ModelHeader::VertexProperies_Normals)), "should contain normal data");
            }

            if (mesh->HasTangentsAndBitangents())
            {
                core::Vector3D tangent;
                tangent.x = mesh->mTangents[v].x;
                tangent.y = mesh->mTangents[v].y;
                tangent.z = mesh->mTangents[v].z;
                meshStream->write<core::Vector3D>(tangent);
#ifdef DEBUG
                memorySize += sizeof(core::Vector3D);
#endif //DEBUG
                if (m_useBitangents)
                {
                    core::Vector3D bitangent;
                    bitangent.x = mesh->mBitangents[v].x;
                    bitangent.y = mesh->mBitangents[v].y;
                    bitangent.z = mesh->mBitangents[v].z;
                    meshStream->write<core::Vector3D>(bitangent);
#ifdef DEBUG
                    memorySize += sizeof(core::Vector3D);
#endif //DEBUG
                }
            }
            else
            {
                ASSERT(!(m_headerRules && ((m_header._vertexContentFlags & scene::ModelHeader::VertexProperies_Tangent) ||
                    (m_header._vertexContentFlags & scene::ModelHeader::VertexProperies_Bitangent))), "should contain tangent or/and bitanget data");
            }

            for (u32 uv = 0; uv < scene::k_maxTextureCoordsIndex; uv++)
            {
                if (mesh->HasTextureCoords(uv))
                {
                    core::Vector2D coord;
                    coord.x = mesh->mTextureCoords[uv][v].x;
                    coord.y = mesh->mTextureCoords[uv][v].y;
                    meshStream->write<core::Vector2D>(coord);
#ifdef DEBUG
                    memorySize += sizeof(core::Vector2D);
#endif //DEBUG
                }
                else
                {
                    ASSERT(!(m_headerRules && (m_header._vertexContentFlags & scene::ModelHeader::VertexProperies_TextCoord0 + uv)), "should contain texture coord data");
                }
            }

            for (u32 c = 0; c < scene::k_maxVertexColorIndex; c++)
            {
                if (mesh->HasVertexColors(c))
                {
                    core::Vector4D color;
                    color.x = mesh->mColors[c][v].r;
                    color.y = mesh->mColors[c][v].g;
                    color.z = mesh->mColors[c][v].b;
                    color.w = mesh->mColors[c][v].a;
                    meshStream->write<core::Vector4D>(color);
#ifdef DEBUG
                    memorySize += sizeof(core::Vector4D);
#endif //DEBUG
                }
                else
                {
                    ASSERT(!(m_headerRules && (m_header._vertexContentFlags & scene::ModelHeader::VertexProperies_Color0 + c)), "should contain color data");
                }
            }
        }

#ifdef DEBUG
        ASSERT(meshSize == memorySize, "different sizes");
#endif //DEBUG
        meshStream->seekBeg(0);

        void* data = meshStream->map(static_cast<u32>(meshSize));
        modelStream->write(data, static_cast<u32>(meshSize));
        meshStream->unmap();

        delete meshStream;

        bool skipIndex = /*m_headerRules && */!m_generateIndices;
        if (!skipIndex)
        {
            if (m_seperateMesh)
            {
                indexBuffer.clear();
                indexBuffer.reserve(newHeader->_meshes[m]._index._count * 3);
            }

            u32 indexBase = static_cast<u32>(indexBuffer.size());
            u32 indexCount = 0;
            /*for (u32 f = 0; f < scene->mMeshes[m]->mNumFaces; f++)
            {
                for (u32 i = 0; i < 3; i++)
                {
                    indexBuffer.push_back(scene->mMeshes[m]->mFaces[f].mIndices[i] + indexBase);
                    indexCount++;
                }
            }*/
            for (u32 f = 0; f < scene->mMeshes[m]->mNumFaces; f++)
            {
                aiFace& face = scene->mMeshes[m]->mFaces[f];
                for (u32 i = 0; i < face.mNumIndices; i++)
                {
                    if (m_seperateMesh)
                    {
                        indexBuffer.push_back(face.mIndices[i]);
                    }
                    else
                    {
                        indexBuffer.push_back(scene->mMeshes[m]->mFaces[f].mIndices[i] + indexBase);
                    }
                    indexCount++;
                }
            }

            u32 indexSize = indexCount * sizeof(u32);
            if (m_seperateMesh)
            {
                modelStream->write(indexBuffer.data(), indexSize, 1);

                newHeader->_meshes[m]._size += indexSize;
                newHeader->_meshes[m]._offset += globalIndexSize;

                scene::MeshHeader::GeometryInfo& indexInfo = newHeader->_meshes[m]._index;
                indexInfo._size = indexSize;
                indexInfo._count = indexCount;
                indexInfo._subData.push_back({ globalIndexSize, indexSize, indexCount });
            }
            else
            {
                newHeader->_meshes.front()._size += indexSize;

                scene::MeshHeader::GeometryInfo& indexInfo = newHeader->_meshes.front()._index;
                indexInfo._size += indexSize;
                indexInfo._count += indexCount;
                indexInfo._subData.push_back({ globalIndexSize, indexSize, indexCount });
            }
            globalIndexSize += indexSize;
        }
    }

    if (!m_seperateMesh && m_generateIndices)
    {
        modelStream->write(indexBuffer.data(), static_cast<u32>(globalIndexSize), 1);
    }

    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: load meshes: %d, size %d bytes", scene->mNumMeshes, globalVertexSize + globalIndexSize);
    return true;
}

bool MeshAssimpDecoder::decodeMaterial(const aiScene * scene, stream::Stream * stream, scene::ModelHeader * newHeader)
{
    newHeader->_materials.resize(scene->mNumMaterials);
    for (u32 m = 0; m < scene->mNumMaterials; m++)
    {
        aiMaterial* material = scene->mMaterials[m];
        scene::MaterialHeader& materialHeader = newHeader->_materials[m];

#if DEBUG
        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        materialHeader._debugName = name.C_Str();
        LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: Load material index %d, name %s", m, name.C_Str());
#endif

        std::tuple<std::string, aiTextureType, scene::MaterialHeader::Property, bool> vectorProp[] =
        {
            { "$clr.diffuse", aiTextureType_DIFFUSE, scene::MaterialHeader::Property_Diffuse, true },
            { "$clr.ambient", aiTextureType_AMBIENT, scene::MaterialHeader::Property_Ambient, true },
            { "$clr.specular", aiTextureType_SPECULAR, scene::MaterialHeader::Property_Specular, true },
            { "$clr.emissive", aiTextureType_EMISSIVE, scene::MaterialHeader::Property_Emission, true },

            { "", aiTextureType_NORMALS, scene::MaterialHeader::Property_Normals, false },
            { "", aiTextureType_HEIGHT, scene::MaterialHeader::Property_Heightmap, false },

            { "$mat.shininess", aiTextureType_SHININESS, scene::MaterialHeader::Property_Shininess, false },
            { "$mat.opacity", aiTextureType_OPACITY, scene::MaterialHeader::Property_Opacity, false },
        };

        for (auto& iter : vectorProp)
        {
            scene::MaterialHeader::PropertyInfo info;

            aiReturn result = aiReturn_FAILURE;
            if (std::get<3>(iter))
            {
                aiColor4D color;

                result = material->Get(std::get<0>(iter).c_str(), 0, 0, color);
                if (result == aiReturn_SUCCESS)
                {
                   core::Vector4D value;
                   value.x = color.r;
                   value.y = color.g;
                   value.z = color.b;
                   value.w = color.a;

                   info._value = value;
                }
            }
            else
            {
                f32 value;
                result = material->Get(std::get<0>(iter).c_str(), 0, 0, value);
                if (result == aiReturn_SUCCESS)
                {
                    info._value = value;
                }
            }

            bool texturePresent = material->GetTextureCount(std::get<1>(iter)) > 0;
            if (texturePresent)
            {
                aiString texture;
                material->GetTexture(std::get<1>(iter), 0, &texture);
                info._name = texture.C_Str();
            }

            if (result == aiReturn_SUCCESS || texturePresent)
            {
                materialHeader._properties.emplace(std::make_pair(std::get<2>(iter), info));
            }
        }
    }

    LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: load materials: %d", newHeader->_materials.size());
    return true;
}

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP
