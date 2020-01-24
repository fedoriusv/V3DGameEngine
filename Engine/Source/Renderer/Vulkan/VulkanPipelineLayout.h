#pragma once

#include "Common.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/ShaderProperties.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    constexpr u32 k_maxDescriptorSetIndex = 4;
    constexpr u32 k_maxDescriptorBindingIndex = 8;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using DescriptorSetLayouts = std::array<VkDescriptorSetLayout, k_maxDescriptorSetIndex>;

    /**
    * VulkanPipelineLayout struct. Vulkan Render side
    */
    struct VulkanPipelineLayout
    {
        VulkanPipelineLayout() noexcept;

        u64 _key;
        VkPipelineLayout _layout;
        DescriptorSetLayouts _descriptorSetLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanPipelineLayoutManager class. Vulkan Render side
    */
    class VulkanPipelineLayoutManager final
    {
    public:

        struct DescriptorSetDescription
        {
            DescriptorSetDescription(const std::array<const Shader*, ShaderType::ShaderType_Count>& shaders) noexcept;

            DescriptorSetDescription() = delete;
            DescriptorSetDescription(const DescriptorSetDescription&) = delete;

            u64 _hash;
            std::array<std::vector<VkDescriptorSetLayoutBinding>, k_maxDescriptorSetIndex> _descriptorSets;
            std::vector<VkPushConstantRange> _pushConstant;
        };

        static VkShaderStageFlagBits convertShaderTypeToVkStage(ShaderType type);

        explicit VulkanPipelineLayoutManager(VkDevice device) noexcept;
        ~VulkanPipelineLayoutManager();

        VulkanPipelineLayoutManager() = delete;
        VulkanPipelineLayoutManager(const VulkanPipelineLayoutManager&) = delete;

        VulkanPipelineLayout acquirePipelineLayout(const DescriptorSetDescription& desc);
        bool removePipelineLayout(const DescriptorSetDescription& desc);
        bool removePipelineLayout(VulkanPipelineLayout& layout);

        void clear();

    private:

        VkPipelineLayout createPipelineLayout(const DescriptorSetDescription& desc, DescriptorSetLayouts& descriptorSetLayouts);
        void destroyPipelineLayout(VkPipelineLayout layout, DescriptorSetLayouts& descriptorSetLayouts);

        bool createDescriptorSetLayouts(const DescriptorSetDescription& desc, DescriptorSetLayouts& descriptorSetLayouts);
        void destroyDescriptorSetLayouts(DescriptorSetLayouts& descriptorSetLayouts);


        VkDevice m_device;

        std::unordered_map<u64, VulkanPipelineLayout> m_pipelinesLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
