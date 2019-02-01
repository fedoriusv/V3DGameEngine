#pragma once

#include "Common.h"
#include "Renderer/ShaderProperties.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxDescriptorSetIndex = 4;
    constexpr u32 k_maxDescriptorBindingIndex = 8;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanPipelineLayout struct. Vulkan Render side
    */
    struct VulkanPipelineLayout
    {
        VulkanPipelineLayout();

        u64 _key;
        VkPipelineLayout                    _layout;
        std::vector<VkDescriptorSetLayout>  _descriptorSetLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanDescriptorPool class. Vulkan Render side
    */
    class VulkanDescriptorPool
    {
    public:
        VulkanDescriptorPool(VkDevice device, VkDescriptorPoolCreateFlags flag) noexcept;

        bool create(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes);
        void destroy();

        bool reset(VkDescriptorPoolResetFlags flag);

        bool allocateDescriptorSet(const VulkanPipelineLayout& layout, std::vector<VkDescriptorSet>& descriptorSets);
        bool freeDescriptorSet(std::vector<VkDescriptorSet>& descriptorSets);

    private:

        bool createDescriptorPool(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes);

        VkDevice m_device;
        VkDescriptorPoolCreateFlags m_flag;
        std::vector<VkDescriptorSet> m_descriptorSets;

        VkDescriptorPool m_pool;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanDescriptorSetManager class. Vulkan Render side
    */
    class VulkanDescriptorSetManager final
    {
    public:

        struct DescriptorSetDescription
        {
            DescriptorSetDescription(const std::array<resource::Shader*, ShaderType::ShaderType_Count>& shaders) noexcept;
            DescriptorSetDescription() = delete;
            DescriptorSetDescription(const DescriptorSetDescription&) = delete;

            u64 _hash;
            std::array<std::vector<VkDescriptorSetLayoutBinding>, k_maxDescriptorSetIndex> _descriptorSets;
            std::vector<VkPushConstantRange> _pushConstant;
        };

        static VkShaderStageFlagBits convertShaderTypeToVkStage(ShaderType type);

        VulkanDescriptorSetManager(VkDevice device) noexcept;
        VulkanDescriptorSetManager() = delete;
        VulkanDescriptorSetManager(const VulkanDescriptorSetManager&) = delete;

        VulkanPipelineLayout acquirePipelineLayout(const DescriptorSetDescription& desc);
        bool removePipelineLayout(const DescriptorSetDescription& desc);
        bool removePipelineLayout(VulkanPipelineLayout& layout);

        VulkanDescriptorPool* acquireDescriptorSets(const VulkanPipelineLayout& layout, std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets);

    private:

        VkPipelineLayout createPipelineLayout(const DescriptorSetDescription& desc, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void destroyPipelineLayout(VkPipelineLayout layout, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

        bool createDescriptorSetLayouts(const DescriptorSetDescription& desc, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void destroyDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

        VulkanDescriptorPool* createPool(const VulkanPipelineLayout& layout, VkDescriptorPoolCreateFlags flag);

        VkDevice m_device;

        std::map<u64, VulkanPipelineLayout> m_pipelinesLayouts;
        std::deque<VulkanDescriptorPool*> m_descriptorPools;

        static std::vector<VkDescriptorPoolSize> s_poolSizes;
        static u32 s_maxSets;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
