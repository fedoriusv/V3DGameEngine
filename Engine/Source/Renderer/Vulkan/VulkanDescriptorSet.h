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

    class DescriptorSetManager
    {
    public:

        static VkShaderStageFlagBits convertShaderTypeToVkStage(resource::ShaderType type);

        DescriptorSetManager(VkDevice device) noexcept;

        void createPipelineLayout();
        void removePipelineLayout();

        VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<resource::Shader*> shaders);
        void removeDescriptorSetLayout();

    private:

        VkDevice m_device;

        std::map<u32, VkDescriptorSetLayout> m_descriptorSetLayouts;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
