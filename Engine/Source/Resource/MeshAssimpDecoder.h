#pragma once

#include "ResourceDecoder.h"
#include "Scene/Geometry/Model.h"
#include "Resource/ModelFileLoader.h"

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
    * Support formats: "dae", "fbx"
    */
    class MeshAssimpDecoder final : public ResourceDecoder
    {
    public:

        explicit MeshAssimpDecoder(std::vector<std::string> supportedExtensions, const scene::ModelHeader& header, ModelFileLoader::ModelLoaderFlags flags) noexcept;
        ~MeshAssimpDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        u32 decodeMesh(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header, u32 activeFlags) const;
        bool decodeMaterial(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header) const;
        bool decodeAABB(const aiScene* scene, stream::Stream* stream, scene::ModelHeader* header) const;

        mutable scene::ModelHeader m_header;
        bool m_headerRules;

        bool m_skipIndices;
        bool m_skipNormals;
        bool m_skipTangents;
        bool m_skipTextureCoords;

        bool m_seperatePosition;
        bool m_generateBoundingBox;

        bool m_localTransform;
        bool m_flipYPosition;
        bool m_flipYTexCoord;

        bool m_splitLargeMeshes;
        bool m_skipMaterialLoading;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP