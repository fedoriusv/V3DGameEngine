#include "MeshAssimpDecoder.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/PipelineStateProperties.h"
#include "Renderer/Formats.h"
#include "Stream/StreamManager.h"
#include "Scene/Material.h"

#include "Utils/Logger.h"

#ifdef USE_ASSIMP
#   include <assimp/Importer.hpp>
#   include <assimp/scene.h>
#   include <assimp/postprocess.h>
#   include <assimp/cimport.h>
#endif //USE_ASSIMP

namespace v3d
{
namespace resource
{

MeshAssimpDecoder::MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModelHeader& header, bool readHeader) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_headerRules(readHeader)
{
}

MeshAssimpDecoder::~MeshAssimpDecoder()
{
}

Resource * MeshAssimpDecoder::decode(const stream::Stream* stream, const std::string& name)
{
    if (stream->size() > 0)
    {
#ifdef USE_ASSIMP
        stream->seekBeg(0);

        const aiScene* scene;
        Assimp::Importer Importer;

        scene::ModelHeader* newHeader = new scene::ModelHeader(m_header);
        u32 assimpFlags = 0;
        if (m_headerRules)
        {
            assimpFlags = aiProcess_MakeLeftHanded | aiProcess_FlipUVs;
            if (!m_header._localTransform)
            {
                assimpFlags |= aiProcess_PreTransformVertices;
            }

            if (m_header._mode == renderer::PolygonMode::PolygonMode_Triangle)
            {
                assimpFlags |= aiProcess_Triangulate;
            }

            if (m_header._frontFace == renderer::FrontFace::FrontFace_Clockwise)
            {
                assimpFlags |= aiProcess_FlipWindingOrder;
            }
            newHeader->_modelContent |= scene::ModelHeader::ModelContext_Mesh;
        }
        else
        {
            assimpFlags = aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_PreTransformVertices;

            newHeader->_vertexContent = 0;
            newHeader->_mode = renderer::PolygonMode::PolygonMode_Triangle;
            newHeader->_frontFace = renderer::FrontFace::FrontFace_Clockwise;
            newHeader->_localTransform = false;
        }

        u8* data = stream->map(stream->size());
        scene = Importer.ReadFileFromMemory(data, stream->size(), assimpFlags);
        if (!scene)
        {
            ASSERT(false, "nullptr");
            delete newHeader;

            return nullptr;
        }

        stream::Stream* modelStream = stream::StreamManager::createMemoryStream();

        if (scene->HasMeshes())
        {
            if (m_headerRules && (newHeader->_modelContent & scene::ModelHeader::ModelContext_Mesh))
            {
                MeshAssimpDecoder::decodeMesh(scene, modelStream, newHeader);
            }
            else
            {
                MeshAssimpDecoder::decodeMesh(scene, modelStream, newHeader);
                newHeader->_modelContent |= scene::ModelHeader::ModelContext_Mesh;
            }
        }

        if (scene->HasMaterials())
        {
            if (m_headerRules && (newHeader->_modelContent & scene::ModelHeader::ModelContext_Material))
            {
                MeshAssimpDecoder::decodeMaterial(scene, modelStream, newHeader);
            }
            else
            {
                MeshAssimpDecoder::decodeMaterial(scene, modelStream, newHeader);
                newHeader->_modelContent |= scene::ModelHeader::ModelContext_Material;
            }
        }

        scene::Model* model = new scene::Model(newHeader);
        model->init(modelStream);

        return model;
#endif
        ASSERT(false, "not implemented");
    }

    return nullptr;
}

bool MeshAssimpDecoder::decodeMesh(const aiScene* scene, stream::Stream* modelStream, scene::ModelHeader* newHeader)
{
    u64 globalModelSize = 0;
    newHeader->_vertex._present = true;
    newHeader->_vertex._countElements = scene->mNumMeshes;
    newHeader->_vertex._data.reserve(scene->mNumMeshes);

    std::vector<renderer::VertexInputAttribDescription> attribDescriptionList;

    for (u32 m = 0; m < scene->mNumMeshes; m++)
    {
        std::vector<renderer::VertexInputAttribDescription::InputBinding> inputBindings;
        std::vector<renderer::VertexInputAttribDescription::InputAttribute> inputAttributes;

        scene::ModelHeader::VertexProperiesFlags contentFlag = 0;
        auto buildVertexData = [&inputBindings, &inputAttributes, &contentFlag](const aiMesh* mesh, const scene::ModelHeader& header) -> u32
        {
            u32 vertexSize = 0;
            if (mesh->HasPositions())
            {
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
                renderer::VertexInputAttribDescription::InputAttribute attrib;
                attrib._bindingId = 0;
                attrib._streamId = 0;
                attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                attrib._offest = vertexSize;
                inputAttributes.push_back(attrib);

                contentFlag |= scene::ModelHeader::VertexProperies_Tangent;
                vertexSize += sizeof(core::Vector3D);
            }

            for (u32 uv = 0; uv < scene::k_maxTextureCoordsIndex; uv++)
            {
                if (mesh->HasTextureCoords(uv))
                {
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
        u32 stride = buildVertexData(mesh, m_header);
        u64 meshSize = stride * mesh->mNumVertices;
        ASSERT(stride > 0, "invalid stride");

        newHeader->_vertex._data.push_back({ meshSize , globalModelSize, mesh->mNumVertices });
        newHeader->_vertexContent = contentFlag;
        globalModelSize += meshSize;

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
                position.y = mesh->mVertices[v].y;
                position.z = mesh->mVertices[v].z;
                meshStream->write<core::Vector3D>(position);
#ifdef DEBUG
                memorySize += sizeof(core::Vector3D);
#endif //DEBUG
            }
            else
            {
                ASSERT(!(m_headerRules && (m_header._vertexContent & scene::ModelHeader::VertexProperies_Position)), "should contain vertex data");
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
                ASSERT(!(m_headerRules && (m_header._vertexContent & scene::ModelHeader::VertexProperies_Normals)), "should contain normal data");
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
            }
            else
            {
                ASSERT(!(m_headerRules && (m_header._vertexContent & scene::ModelHeader::VertexProperies_Tangent)), "should contain tangent data");
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
                    ASSERT(!(m_headerRules && (m_header._vertexContent & scene::ModelHeader::VertexProperies_TextCoord0 + uv)), "should contain texture coord data");
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
                    ASSERT(!(m_headerRules && (m_header._vertexContent & scene::ModelHeader::VertexProperies_Color0 + c)), "should contain color data");
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

        inputBindings.push_back(renderer::VertexInputAttribDescription::InputBinding(0, renderer::VertexInputAttribDescription::InputRate_Vertex, stride));
        attribDescriptionList.emplace_back(inputBindings, inputAttributes);

        delete meshStream;
    }
    newHeader->_vertex._globalSize = globalModelSize;

    bool skipIndex = m_headerRules && !newHeader->_index._present;
    if (!skipIndex)
    {
        std::vector<u32> indexBuffer;
        for (u32 m = 0; m < scene->mNumMeshes; m++)
        {
            u32 indexBase = static_cast<u32>(indexBuffer.size());
            u32 indexSize = 0;
            u32 indexCount = 0;
            for (u32 f = 0; f < scene->mMeshes[m]->mNumFaces; f++)
            {
                for (u32 i = 0; i < 3; i++)
                {
                    indexBuffer.push_back(scene->mMeshes[m]->mFaces[f].mIndices[i] + indexBase);
                    indexSize += sizeof(u32);
                    indexCount++;
                }
            }

            newHeader->_index._data.push_back({ indexSize , static_cast<u32>(indexBuffer.size() * sizeof(u32)), indexCount });
        }

        newHeader->_index._present = true;
        newHeader->_index._countElements = static_cast<u32>(indexBuffer.size());
        newHeader->_index._globalSize = static_cast<u32>(indexBuffer.size()) * sizeof(u32);

        modelStream->write(indexBuffer.data(), static_cast<u32>(newHeader->_index._globalSize));
    }
    else
    {
        newHeader->_index._present = false;
    }

    for (auto& desc : attribDescriptionList)
    {
        desc >> modelStream;
    }

    LOG_DEBUG("MeshAssimpDecoder::decodeMesh: load meshes: %d, size %d bytes", newHeader->_vertex._countElements, newHeader->_vertex._globalSize);
    return true;
}

bool MeshAssimpDecoder::decodeMaterial(const aiScene * scene, stream::Stream * stream, scene::ModelHeader * newHeader)
{
    newHeader->_materials._countElements = scene->mNumMaterials;
    for (u32 m = 0; m < scene->mNumMaterials; m++)
    {
        stream::Stream* materialStream = stream::StreamManager::createMemoryStream();

        aiMaterial* material = scene->mMaterials[m];

        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        newHeader->_materials._names.push_back(name.C_Str());

        {
            aiColor4D color;
            material->Get(AI_MATKEY_COLOR_AMBIENT, color);

            aiString texturefile;
            if (material->GetTextureCount(aiTextureType_AMBIENT) > 0)
            {

                material->GetTexture(aiTextureType_AMBIENT, 0, &texturefile);
            }

            scene::Material::Ambient ambient;
            ambient._color.x = color.r;
            ambient._color.y = color.g;
            ambient._color.z = color.b;
            ambient._color.w = color.a;
            ambient._texture = texturefile.C_Str();

            ambient >> materialStream;
        }

        {
            aiColor4D color;
            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);

            aiString texturefile;
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {

                material->GetTexture(aiTextureType_DIFFUSE, 0, &texturefile);
            }

            scene::Material::Diffuse diffuse;
            diffuse._color.x = color.r;
            diffuse._color.y = color.g;
            diffuse._color.z = color.b;
            diffuse._color.w = color.a;
            diffuse._texture = texturefile.C_Str();

            diffuse >> materialStream;
        }

        {
            aiColor4D color;
            material->Get(AI_MATKEY_COLOR_SPECULAR, color);

            aiString texturefile;
            if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
            {
                material->GetTexture(aiTextureType_SPECULAR, 0, &texturefile);
            }
        }

        {
            aiColor4D color;
            material->Get(AI_MATKEY_BUMPSCALING, color);

            if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
            {
                aiString texturefile;
                material->GetTexture(aiTextureType_NORMALS, 0, &texturefile);
            }
        }

        {
            aiColor4D color;
            material->Get(AI_MATKEY_OPACITY, color);

            aiString texturefile;
            if (material->GetTextureCount(aiTextureType_OPACITY) > 0)
            {
                material->GetTexture(aiTextureType_NORMALS, 0, &texturefile);
            }
        }

        materialStream->seekBeg(0);
        void* data = materialStream->map(static_cast<u32>(materialStream->size()));
        stream->write(data, static_cast<u32>(materialStream->size()));
        materialStream->unmap();
    }

    LOG_DEBUG("MeshAssimpDecoder::decodeMaterial: load materials: %d", newHeader->_materials._countElements);

    return false;
}

} //namespace decoders
} //namespace v3d
