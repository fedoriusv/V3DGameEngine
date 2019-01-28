#pragma once

#include "Common.h"
#include "Renderer/ShaderProperties.h"
#include "Resource/Shader.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct DescriptorBinding
    {
        struct BindingInfo
        {
            VkImageView      _imageView;
        };
    };

    struct VulkanPipelineLayout
    {
        VulkanPipelineLayout();

        u32 _key;
        VkPipelineLayout                    _layout;
        std::vector<VkDescriptorSetLayout>  _descriptorSetLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDescriptorSetManager final
    {
    public:

        struct DescriptorSetDescription
        {
            DescriptorSetDescription(const std::vector<resource::Shader*> shaders) noexcept;
            DescriptorSetDescription() = delete;
            DescriptorSetDescription(const DescriptorSetDescription&) = delete;

            u32 _hash;
            std::vector<std::vector<VkDescriptorSetLayoutBinding>> _descriptorSets;
            std::vector<VkPushConstantRange> _pushConstant;
        };

        static VkShaderStageFlagBits convertShaderTypeToVkStage(resource::ShaderType type);

        VulkanDescriptorSetManager(VkDevice device) noexcept;
        VulkanDescriptorSetManager() = delete;
        VulkanDescriptorSetManager(const VulkanDescriptorSetManager&) = delete;

        VulkanPipelineLayout acquirePipelineLayout(const DescriptorSetDescription& desc);
        bool removePipelineLayout(const DescriptorSetDescription& desc);
        bool removePipelineLayout(VulkanPipelineLayout& layout);

    private:

        VkPipelineLayout createPipelineLayout(const DescriptorSetDescription& desc, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void destroyPipelineLayout(VkPipelineLayout layout, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

        bool createDescriptorSetLayouts(const DescriptorSetDescription& desc, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void destroyDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

        VkDevice m_device;

        std::map<u32, VulkanPipelineLayout> m_pipelinesLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
