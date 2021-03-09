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
    /////////////////////////////////////////////////////////////////////////////////////////////////////
   
    /**
    * VulkanPipelineLayoutDescription struct. Vulkan Render side
    */
    struct VulkanDescriptorSetLayoutDescription
    {
        VulkanDescriptorSetLayoutDescription() noexcept;
        VulkanDescriptorSetLayoutDescription(const std::vector<VkDescriptorSetLayoutBinding>& bindings) noexcept;

        struct Equal
        {
            bool operator()(const VulkanDescriptorSetLayoutDescription& descl, const VulkanDescriptorSetLayoutDescription& descr) const;
        };

        struct Hash
        {
            size_t operator()(const VulkanDescriptorSetLayoutDescription& desc) const;
        };

        u64 _key;
        std::vector<VkDescriptorSetLayoutBinding> _bindings;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanPipelineLayoutDescription struct. Vulkan Render side
    */
    struct VulkanPipelineLayoutDescription
    {
        VulkanPipelineLayoutDescription() noexcept;
        
        struct Equal
        {
            bool operator()(const VulkanPipelineLayoutDescription& descl, const VulkanPipelineLayoutDescription& descr) const;
        };

        struct Hash
        {
            size_t operator()(const VulkanPipelineLayoutDescription& desc) const;
        };

        u64 _key;
        std::array<std::vector<VkDescriptorSetLayoutBinding>, k_maxDescriptorSetIndex> _bindingsSet;
        std::vector<VkPushConstantRange> _pushConstant;
    };

    /**
    * VulkanPipelineLayout struct. Vulkan Render side
    */
    struct VulkanPipelineLayout
    {
        VulkanPipelineLayout() noexcept;

        VkPipelineLayout _pipelineLayout;
        std::array<VkDescriptorSetLayout, k_maxDescriptorSetIndex> _setLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanPipelineLayoutManager class. Vulkan Render side
    */
    class VulkanPipelineLayoutManager final
    {
    public:

        struct DescriptorSetLayoutCreator
        {
            DescriptorSetLayoutCreator(const std::array<const Shader*, ShaderType::ShaderType_Count>& shaders) noexcept;

            DescriptorSetLayoutCreator() = delete;
            DescriptorSetLayoutCreator(const DescriptorSetLayoutCreator&) = delete;

            VulkanPipelineLayoutDescription _description;
        };

        static VkShaderStageFlagBits convertShaderTypeToVkStage(ShaderType type);

        explicit VulkanPipelineLayoutManager(VkDevice device) noexcept;
        ~VulkanPipelineLayoutManager();

        VulkanPipelineLayoutManager() = delete;
        VulkanPipelineLayoutManager(const VulkanPipelineLayoutManager&) = delete;

        VulkanPipelineLayout acquirePipelineLayout(const VulkanPipelineLayoutDescription& desc);
        bool removePipelineLayout(const VulkanPipelineLayoutDescription& desc);

        void clear();

    private:

        VkDescriptorSetLayout acquireDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

        VkPipelineLayout createPipelineLayout(const VulkanPipelineLayoutDescription& desc, const std::array<VkDescriptorSetLayout, k_maxDescriptorSetIndex>& descriptorSetLayouts);

        VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        void destroyDescriptorSetLayouts(std::array<VkDescriptorSetLayout, k_maxDescriptorSetIndex>& descriptorSetLayouts);

        VkDevice m_device;
        std::unordered_map<VulkanDescriptorSetLayoutDescription, VkDescriptorSetLayout, VulkanDescriptorSetLayoutDescription::Hash, VulkanDescriptorSetLayoutDescription::Equal> m_descriptorSetLayouts;
        std::unordered_map<VulkanPipelineLayoutDescription, VulkanPipelineLayout, VulkanPipelineLayoutDescription::Hash, VulkanPipelineLayoutDescription::Equal> m_pipelinesLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
