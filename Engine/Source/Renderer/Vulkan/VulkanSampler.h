#pragma once

#include "Common.h"
#include "Renderer/SamplerState.h"

#ifdef VULKAN_RENDER
# include "VulkanWrapper.h"
# include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanSampler class. Vulkan Render side
    */
    class VulkanSampler final : public Sampler, public VulkanResource
    {
    public:

        static VkFilter convertSamplerFilterToVk(SamplerFilter filter);
        static VkSamplerMipmapMode convertMipmapSamplerFilterToVk(SamplerFilter filter);
        static f32 convertAnisotropyCount(SamplerAnisotropic level);
        static VkSamplerAddressMode convertSamplerWrapToVkSamplerAddressMode(SamplerWrap wrap);

        explicit VulkanSampler(VulkanDevice* device, const SamplerDesc& desc, const std::string& name = "") noexcept;
        ~VulkanSampler();

        bool create() override;
        void destroy() override;

        VkSampler getHandle() const;

    private:

        VulkanSampler() = delete;
        VulkanSampler(const VulkanSampler&) = delete;

        VulkanDevice& m_device;

        SamplerDesc m_desc;
        VkSampler m_sampler;
#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline VkSampler VulkanSampler::getHandle() const
    {
        ASSERT(m_sampler, "nullptr");
        return m_sampler;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
