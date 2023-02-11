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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDescriptorSetPool;
    class VulkanDescriptorPoolProvider;

    /**
    * @brief BindingType enum. Vulkan Render side
    */
    enum BindingType : u32
    {
        BindingType_Unknown = 0,
        BindingType_Uniform,
        BindingType_DynamicUniform,
        BindingType_Sampler,
        BindingType_Texture,
        BindingType_SamplerAndTexture,
        BindingType_StorageImage,
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
        };


        DescriptorInfo  _info;            //24
        BindingType     _type       : 16;
        u32             _binding    : 16; //28
        u32             _arrayIndex : 16;

        u32             _padding    : 16; //32
    };

    /**
    * @brief SetInfo struct. Vulkan Render side
    */
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

        u64 _key = 0;
        u64 _size = 0;
        std::array<BindingInfo, k_maxDescriptorBindingIndex> _bindingsInfo;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDescriptorSetManager class. Vulkan Render side
    */
    class VulkanDescriptorSetManager final
    {
    public:

        explicit VulkanDescriptorSetManager(VkDevice device, u32 swapchainImages) noexcept;
        ~VulkanDescriptorSetManager();

        //DescriptorSets
        VkDescriptorSet acquireDescriptorSet(const VulkanDescriptorSetLayoutDescription& desc, const SetInfo& info, VkDescriptorSetLayout layoutSet, VulkanDescriptorSetPool*& pool);

        void updateDescriptorPools();

    private:

        VulkanDescriptorSetManager() = delete;
        VulkanDescriptorSetManager(const VulkanDescriptorSetManager&) = delete;

        VulkanDescriptorPoolProvider* m_poolProvider;
        std::vector<VulkanDescriptorSetPool*> m_currentPool;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
