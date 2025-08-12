#pragma once

#include "ResourceDecoder.h"
#include "Resource/Loader/ModelFileLoader.h"

#ifdef USE_ASSIMP
struct aiScene;

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief AssimpDecoder decoder.
    * Support formats: "dae", "fbx"
    */
    class AssimpDecoder final : public ResourceDecoder
    {
    public:

        explicit AssimpDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit AssimpDecoder(std::vector<std::string>&& supportedExtensions) noexcept;
        ~AssimpDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const Policy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:

        u32 decodeMesh(const aiScene* scene, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags, u32 vertexPropFlags) const;
        u32 decodeSkeleton(const aiScene* scene, stream::Stream* stream) const;
        u32 decodeMaterial(const aiScene* scene, stream::Stream* stream) const;
        u32 decodeLight(const aiScene* scene, stream::Stream* stream) const;
        u32 decodeCamera(const aiScene* scene, stream::Stream* stream) const;

        mutable std::vector<std::tuple<u32, u32>> m_materialMapper;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP