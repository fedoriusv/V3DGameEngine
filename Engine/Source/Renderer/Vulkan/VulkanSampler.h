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

        explicit VulkanSampler(VulkanDevice* device, const std::string& name = "") noexcept;
        ~VulkanSampler();

        bool create(const SamplerDesc& desc) override;
        void destroy() override;

        VkSampler getHandle() const;

    private:

        VulkanSampler(const VulkanSampler&) = delete;
        VulkanSampler& operator=(const VulkanSampler&) = delete;

        VulkanDevice&   m_device;
        VkSampler       m_sampler;
#if VULKAN_DEBUG_MARKERS
        std::string     m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline VkSampler VulkanSampler::getHandle() const
    {
        ASSERT(m_sampler, "nullptr");
        return m_sampler;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SamplerManager class
    * Multithreaded
    */
    class VulkanSamplerManager final
    {
    public:

        explicit VulkanSamplerManager(VulkanDevice* device) noexcept;
        ~VulkanSamplerManager();

        [[nodiscard]] VulkanSampler* acquireSampler(const SamplerState& state);
        bool removeSampler(VulkanSampler* sampler);
        void clear();

    private:

        VulkanSamplerManager(const VulkanSamplerManager&) = delete;
        VulkanSamplerManager& operator=(const VulkanSamplerManager&) = delete;

        VulkanDevice&   m_device;
        std::mutex      m_mutex;
        std::unordered_map<DescInfo<SamplerDesc>, VulkanSampler*, DescInfo<SamplerDesc>::Hash, DescInfo<SamplerDesc>::Compare> m_samplerList;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
