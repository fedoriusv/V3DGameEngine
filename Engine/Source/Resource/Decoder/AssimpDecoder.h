#pragma once

#include "ResourceDecoder.h"
#include "Resource/Loader/ModelFileLoader.h"
#include "Scene/Material.h"

#ifdef USE_ASSIMP
struct aiScene;
struct aiNode;
struct aiMesh;

namespace v3d
{
namespace renderer
{
    class Device;
} // namespace renderer
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

        explicit AssimpDecoder(renderer::Device* device, const std::vector<std::string>& supportedExtensions) noexcept;
        explicit AssimpDecoder(renderer::Device* device, std::vector<std::string>&& supportedExtensions) noexcept;
        ~AssimpDecoder();

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:

        u32 decodeNode(const aiScene* scene, const aiNode* node, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags, u32 vertexPropFlags) const;
        u32 decodeMesh(const aiScene* scene, const aiMesh* mesh, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags, u32 vertexPropFlags) const;
        u32 decodeMaterial(const aiScene* scene, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags, scene::MaterialShadingModel overridedShadingModel) const;

        u32 decodeSkeleton(const aiScene* scene, stream::Stream* stream) const;
        u32 decodeLight(const aiScene* scene, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags) const;
        u32 decodeCamera(const aiScene* scene, stream::Stream* stream, ModelFileLoader::ModelLoaderFlags flags) const;

        renderer::Device* const m_device;
        mutable std::vector<std::tuple<u32, u32>> m_materialMapper;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
#endif //USE_ASSIMP