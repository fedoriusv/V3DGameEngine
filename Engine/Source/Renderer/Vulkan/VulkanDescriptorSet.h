#pragma once

#include "Renderer/Render.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanPipelineLayout.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanResource;
    class VulkanDescriptorSetPool;
    class VulkanDescriptorPoolProvider;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief BindingType enum. Vulkan Render side
    */
    enum class BindingType : u32
    {
        Unknown = 0,
        Uniform,
        DynamicUniform,
        Sampler,
        Texture,
        RWTexture,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief BindingInfo struct. Vulkan Render side
    */
    struct BindingInfo
    {
        BindingInfo() noexcept;

        union DescriptorInfo
        {
            VkDescriptorImageInfo   _imageInfo;
            VkDescriptorBufferInfo  _bufferInfo;
            VkBufferView            _bufferView;
        };

        DescriptorInfo  _info;            //24
        BindingType     _type       : 16;
        u32             _binding    : 16; //28
        u32             _arrayIndex : 16;
        u32             _padding    : 16; //32
    };

    inline BindingInfo::BindingInfo() noexcept
    {
        memset(this, 0, sizeof(BindingInfo));
    }

    /**
    * @brief SetInfo struct. Vulkan Render side
    */
    struct SetInfo
    {
        struct Hash
        {
            size_t operator()(const SetInfo& set) const;
        };

        struct Compare
        {
            bool operator()(const SetInfo& set0, const SetInfo& set1) const;
        };

        bool isActiveBinding(u32 binding) const;
        void clear();

        void calcHash() const;

        mutable u32 _key = 0;
        u32 _activeBindingsFlags = 0;
        std::array<BindingInfo, k_maxDescriptorBindingCount>     _bindings = {};
        std::array<VulkanResource*, k_maxDescriptorBindingCount> _resource = {};
    };

    inline size_t SetInfo::Hash::operator()(const SetInfo& set) const
    {
        ASSERT(set._key, "empty");
        return set._key;
    }

    inline bool SetInfo::Compare::operator()(const SetInfo& set0, const SetInfo& set1) const
    {
        ASSERT(set0._key && set1._key, "must be valid");
        if (set0._key != set1._key)
        {
            return false;
        }

        if (!set0._bindings.empty() && memcmp(set0._bindings.data(), set1._bindings.data(), sizeof(BindingInfo) * set0._bindings.size()) != 0)
        {
            return false;
        }

        return true;
    }

    inline bool SetInfo::isActiveBinding(u32 binding) const
    {
        return (_activeBindingsFlags >> binding) & 1;
    }

    inline void SetInfo::clear()
    {
        _key = 0;
        _activeBindingsFlags = 0;
        _bindings.fill({});
        _resource.fill(nullptr);
    }

    inline void SetInfo::calcHash() const
    {
        u32 count = k_maxDescriptorBindingCount;
        _key = crc32c::Crc32c(reinterpret_cast<const u8*>(_bindings.data()), sizeof(BindingInfo) * count);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    VkDescriptorBufferInfo makeVkDescriptorBufferInfo(const VulkanBuffer* buffer, u64 offset, u64 range);
    VkDescriptorImageInfo makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler, VkImageLayout layout, const RenderTexture::Subresource& subresource);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDescriptorSetManager class. Vulkan Render side
    */
    class VulkanDescriptorSetManager final
    {
    public:

        explicit VulkanDescriptorSetManager(VulkanDevice* device, u32 swapchainImages) noexcept;
        ~VulkanDescriptorSetManager();

        [[nodiscard]] std::tuple<VulkanDescriptorSetPool*, VkDescriptorSet, u32> acquireFreeDescriptorSet(const VulkanDescriptorSetLayoutDescription& desc, VkDescriptorSetLayout layoutSet);

        void updateDescriptorSet(VulkanCommandBuffer* cmdBuffer, VkDescriptorSet set, const SetInfo& setInfo);
        void updateStatus();

    private:

        VulkanDescriptorSetManager() = delete;
        VulkanDescriptorSetManager(const VulkanDescriptorSetManager&) = delete;

        VulkanDevice&                   m_device;
        VulkanDescriptorPoolProvider*   m_poolProvider;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
