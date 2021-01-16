#pragma once

#include "ResourceDecoder.h"
#include "Scene/Model.h"

#ifdef USE_ASSIMP
struct aiScene;

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief MeshAssimpDecoder decoder.
    * Support formats: "dae"
    */
    class MeshAssimpDecoder final : public ResourceDecoder
    {
    public:

        MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModelHeader& header, u32 flags) noexcept;
        ~MeshAssimpDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        bool decodeMesh(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header) const;
        bool decodeMaterial(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header) const;

        const scene::ModelHeader m_header;
        bool m_headerRules;

        bool m_seperateMesh;
        bool m_splitLargeMeshes;
        bool m_generateIndices;
        //bool m_generateTextureCoords;
        bool m_generateNormals;
        bool m_generateTangents;
        bool m_useBitangents;
        bool m_localTransform;
        bool m_flipYPosition;
        bool m_flipYTexCoord;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP