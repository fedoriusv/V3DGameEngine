#pragma once

#include "Common.h"
#include "Renderer/Sampler.h"
#include "Renderer/TextureProperties.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanSampler class. Vulkan Render side
    */
    class VulkanSampler final : public Sampler, public VulkanResource
    {
    public:

        static VkFilter convertSamplerFilterToVk(SamplerFilter filter);
        static VkSamplerMipmapMode convertMipmapSamplerFilterToVk(SamplerFilter filter);
        static f32 convertAnisotropyCount(SamplerAnisotropic level);
        static VkSamplerAddressMode convertSamplerWrapToVkSamplerAddressMode(SamplerWrap wrap);

        VulkanSampler() = delete;
        VulkanSampler(const VulkanSampler&) = delete;

        explicit VulkanSampler(VkDevice device, const SamplerDescription& desc) noexcept;
        ~VulkanSampler();

        bool create() override;
        void destroy() override;

        VkSampler getHandle() const;

    private:

        VkDevice  m_device;
        VkSampler m_sampler;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
