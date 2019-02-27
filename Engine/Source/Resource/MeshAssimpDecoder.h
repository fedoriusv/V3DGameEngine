#pragma once

#include "ResourceDecoder.h"
#include "Scene/Model.h"

struct aiScene;

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class MeshAssimpDecoder final : public ResourceDecoder
    {
    public:

        MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModelHeader& header, bool readHeader) noexcept;
        ~MeshAssimpDecoder();

        Resource* decode(const stream::Stream* stream, const std::string& name = "") override;

    private:

        bool decodeMesh(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header);
        bool decodeMaterial(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header);

        const scene::ModelHeader m_header;
        bool m_headerRules;

        bool m_seperateMesh = false;
        bool m_generateIndices = true;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
