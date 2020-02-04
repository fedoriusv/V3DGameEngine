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
    class VulkanImage;
    class VulkanBuffer;
    class VulkanSampler;
    class VulkanUniformBuffer;

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
    * VulkanDescriptorPool class. Vulkan Render side
    */

    class VulkanDescriptorSetPool : public VulkanResource
    {
    public:
        VulkanDescriptorSetPool(VkDevice device, VkDescriptorPoolCreateFlags flag) noexcept;

        bool create(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes);
        void destroy();

        bool reset(VkDescriptorPoolResetFlags flag);

        u64 getCountDescriptorSets() const;
        
        VkDescriptorSet createDescriptorSet(const SetInfo& info, VkDescriptorSetLayout layout);
        VkDescriptorSet getDescriptorSet(const SetInfo& info);

    private:

        bool createDescriptorPool(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes);

        bool allocateDescriptorSets(std::vector<VkDescriptorSetLayout>& layout, std::vector<VkDescriptorSet>& descriptorSets);
        bool freeDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets);

        bool allocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet);
        bool freeDescriptorSet(VkDescriptorSet& descriptorSet);

        VkDevice m_device;

        VkDescriptorPoolCreateFlags m_flag;
        VkDescriptorPool m_pool;

        std::unordered_map<SetInfo, VkDescriptorSet, SetInfo::Hash, SetInfo::Equal> m_descriptorSets;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanDescriptorSetManager class. Vulkan Render side
    */
    class VulkanDescriptorSetManager final
    {
    public:

        explicit VulkanDescriptorSetManager(VkDevice device) noexcept;
        ~VulkanDescriptorSetManager();

        VulkanDescriptorSetManager() = delete;
        VulkanDescriptorSetManager(const VulkanDescriptorSetManager&) = delete;

        //DescriptorSets
        VkDescriptorSet acquireDescriptorSet(const VulkanDescriptorSetLayoutDescription& desc, const SetInfo& info, VkDescriptorSetLayout layoutSet, VulkanDescriptorSetPool*& pool);

        void updateDescriptorPools();
        void clear();

    private:

        VulkanDescriptorSetPool* acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VkDescriptorPoolCreateFlags flag);
        void destroyPools();

        VkDevice m_device;

        std::deque<VulkanDescriptorSetPool*> m_freeDescriptorPools;
        std::deque<VulkanDescriptorSetPool*> m_usedDescriptorPools;
        VulkanDescriptorSetPool* m_currentDescriptorPool;

        static std::vector<VkDescriptorPoolSize> s_poolSizes;
        static u32 s_maxSets;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
