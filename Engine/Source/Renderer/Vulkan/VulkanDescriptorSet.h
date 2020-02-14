#pragma once

#include "Common.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/ShaderProperties.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"
#include "VulkanPipelineLayout.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanDescriptorSetPool;
    class VulkanDescriptorPoolProvider;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum BindingType : u32
    {
        BindingType_Unknown = 0,
        BindingType_Uniform,
        BindingType_DynamicUniform,
        BindingType_Sampler,
        BindingType_Texture,
        BindingType_SamplerAndTexture,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * BindingInfo struct. Vulkan Render side
    */
    struct BindingInfo
    {
        BindingInfo() noexcept;

        union DescriptorInfo
        {
            VkDescriptorImageInfo   _imageInfo;
            VkDescriptorBufferInfo  _bufferInfo;
        };


        DescriptorInfo  _info;            //24
        BindingType     _type       : 16;
        u32             _binding    : 16; //28
        u32             _arrayIndex : 16;

        u32             _padding    : 16; //32
    };

    struct SetInfo
    {
        struct Hash
        {
            size_t operator()(const SetInfo& set) const;
        };

        struct Equal
        {
            bool operator()(const SetInfo& set0, const SetInfo& set1) const;
        };

        u64 _key;
        std::vector<BindingInfo> _bindingsInfo;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
    * VulkanDescriptorSetManager class. Vulkan Render side
    */
    class VulkanDescriptorSetManager final
    {
    public:

        explicit VulkanDescriptorSetManager(VkDevice device, u32 swapchainImages) noexcept;
        ~VulkanDescriptorSetManager();

        VulkanDescriptorSetManager() = delete;
        VulkanDescriptorSetManager(const VulkanDescriptorSetManager&) = delete;

        //DescriptorSets
        VkDescriptorSet acquireDescriptorSet(const VulkanDescriptorSetLayoutDescription& desc, const SetInfo& info, VkDescriptorSetLayout layoutSet, VulkanDescriptorSetPool*& pool);

        void updateDescriptorPools();

    private:

        VkDevice m_device;
        VulkanDescriptorPoolProvider* m_poolProvider;

        std::vector<VulkanDescriptorSetPool*> m_currentPool;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
