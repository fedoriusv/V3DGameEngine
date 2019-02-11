#include "MeshAssimpDecoder.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/Formats.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#endif //USE_ASSIMP

namespace v3d
{
namespace resource
{

MeshAssimpDecoder::MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModleHeader& header) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
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

        // Flags for loading the mesh
        static const int assimpFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices;

        //scene = Importer.ReadFile(name.c_str(), assimpFlags);
        u8* data = stream->map(stream->size());
        scene = Importer.ReadFileFromMemory(data, stream->size(), assimpFlags);
        if (!scene)
        {
            ASSERT(false, "nullptr");
            return nullptr;
        }
        
        stream::MemoryStream* modelStream = stream::StreamManager::createMemoryStream();
        for (u32 m = 0; m < scene->mNumMeshes; m++)
        {
            std::vector<renderer::VertexInputAttribDescription::InputBinding> inputBindings;
            std::vector<renderer::VertexInputAttribDescription::InputAttribute> inputAttributes;

            auto buildVertexData = [&inputBindings, &inputAttributes](const aiMesh* mesh, const scene::ModleHeader& header) -> u32
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
                }

                if (mesh->HasNormals())
                {
                    vertexSize += sizeof(core::Vector3D);

                    renderer::VertexInputAttribDescription::InputAttribute attrib;
                    attrib._bindingId = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                    attrib._offest = vertexSize;

                    inputAttributes.push_back(attrib);
                }

                if (mesh->HasTangentsAndBitangents())
                {
                    vertexSize += sizeof(core::Vector3D);

                    renderer::VertexInputAttribDescription::InputAttribute attrib;
                    attrib._bindingId = 0;
                    attrib._streamId = 0;
                    attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                    attrib._offest = vertexSize;

                    inputAttributes.push_back(attrib);
                }

                for (u32 uv = 0; uv < 4; uv++)
                {
                    if (mesh->HasTextureCoords(uv))
                    {
                        vertexSize += sizeof(core::Vector2D);

                        renderer::VertexInputAttribDescription::InputAttribute attrib;
                        attrib._bindingId = 0;
                        attrib._streamId = 0;
                        attrib._format = renderer::Format::Format_R32G32_SFloat;
                        attrib._offest = vertexSize;

                        inputAttributes.push_back(attrib);
                    }
                }

                for (u32 c = 0; c < 4; c++)
                {
                    if (mesh->HasVertexColors(c))
                    {
                        vertexSize += sizeof(core::Vector3D);

                        renderer::VertexInputAttribDescription::InputAttribute attrib;
                        attrib._bindingId = 0;
                        attrib._streamId = 0;
                        attrib._format = renderer::Format::Format_R32G32B32_SFloat;
                        attrib._offest = vertexSize;

                        inputAttributes.push_back(attrib);
                    }
                }

                return vertexSize;
            };

            const aiMesh* mesh = scene->mMeshes[m];
            u32 stride = buildVertexData(mesh, m_header);
            ASSERT(stride > 0, "invalid stride");

            stream::MemoryStream* meshData = stream::StreamManager::createMemoryStream(nullptr, stride * mesh->mNumVertices);
            for (u32 v = 0; v < mesh->mNumVertices; v++)
            {
                if (mesh->HasPositions())
                {
                    ASSERT(m_header._content & scene::ModleHeader::VertexProperies_Vertex, "should contain vertex data");
                    core::Vector3D position = scene->mMeshes[m]->mVertices[v].x;
                    meshData->write(&position.x, sizeof(core::Vector3D));
                    //meshData->write<core::Vector3D>(position);
                }

                if (mesh->HasNormals())
                {
                    core::Vector3D normal = scene->mMeshes[m]->mNormals[v].x;
                    meshData->write(&normal.x, sizeof(core::Vector3D));
                }

                if (mesh->HasTangentsAndBitangents())
                {
                    core::Vector3D tangent = scene->mMeshes[m]->mTangents[v].x;
                    meshData->write(&tangent.x, sizeof(core::Vector3D));
                }

                //TODO:
            }

            inputBindings.push_back(renderer::VertexInputAttribDescription::InputBinding(0, renderer::VertexInputAttribDescription::InputRate_Vertex, stride));
            renderer::VertexInputAttribDescription vertexDesc(inputBindings, inputAttributes);

            //vertexDesc >> meshData;

            modelStream->write(meshData, meshData->size());
            delete meshData;
        }

        //// Generate index buffer from ASSIMP scene data
        //std::vector<uint32_t> indexBuffer;
        //for (uint32_t m = 0; m < scene->mNumMeshes; m++)
        //{
        //    uint32_t indexBase = static_cast<uint32_t>(indexBuffer.size());
        //    for (uint32_t f = 0; f < scene->mMeshes[m]->mNumFaces; f++)
        //    {
        //        // We assume that all faces are triangulated
        //        for (uint32_t i = 0; i < 3; i++)
        //        {
        //            indexBuffer.push_back(scene->mMeshes[m]->mFaces[f].mIndices[i] + indexBase);
        //        }
        //    }
        //}
        //size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
        //model.indices.count = static_cast<uint32_t>(indexBuffer.size());

        scene::ModleHeader* newHeader = new scene::ModleHeader(m_header);
        scene::Model* model = new scene::Model(newHeader);
        model->init(modelStream);

        return model;
#endif
        ASSERT(false, "not implemented");
    }

    return nullptr;
}

} //namespace decoders
} //namespace v3d
