#pragma once

#include "Renderer/Render.h"
#include "Renderer/Pipeline.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanResource.h"
#   include "VulkanPipelineLayout.h"
#   include "VulkanRenderpass.h"

namespace v3d
{
namespace resource
{
    class Shader;
} //namespace resource
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanComputePipeline final class. Vulkan Render side
    */
    class VulkanComputePipeline final : public RenderPipeline, public VulkanResource
    {
    public:

        explicit VulkanComputePipeline(VulkanDevice* device, VulkanPipelineLayoutManager* pipelineLayoutManager, const std::string& name = "") noexcept;
        ~VulkanComputePipeline();

        VkPipeline getHandle() const;
        VkPipelineLayout getPipelineLayoutHandle() const;

        const VulkanPipelineLayout& getDescriptorSetLayouts() const;
        const VulkanPipelineLayoutDescription& getPipelineLayoutDescription() const;

        bool create(const ComputePipelineState& state);
        void destroy();

    private:

        VulkanComputePipeline() = delete;
        VulkanComputePipeline(const VulkanComputePipeline&) = delete;

        bool createShaderModules(const renderer::ShaderProgram* program);
        void deleteShaderModules();

        VulkanDevice&                       m_device;
        VulkanPipelineLayoutManager* const  m_pipelineLayoutManager;

        VkPipeline                          m_pipeline;
        VkShaderModule                      m_module;

        VulkanPipelineLayoutDescription     m_pipelineLayoutDescription;
        VulkanPipelineLayout                m_pipelineLayout;

        bool pipelineStatistic() const;

#if VULKAN_DEBUG_MARKERS
        std::string m_debugName;
#endif //VULKAN_DEBUG_MARKERS
    };

    inline const VulkanPipelineLayout& VulkanComputePipeline::getDescriptorSetLayouts() const
    {
        return m_pipelineLayout;
    }

    inline const VulkanPipelineLayoutDescription& VulkanComputePipeline::getPipelineLayoutDescription() const
    {
        return m_pipelineLayoutDescription;
    }

    inline VkPipeline VulkanComputePipeline::getHandle() const
    {
        ASSERT(m_pipeline != VK_NULL_HANDLE, "nullptr");
        return m_pipeline;
    }

    inline VkPipelineLayout VulkanComputePipeline::getPipelineLayoutHandle() const
    {
        ASSERT(m_pipelineLayout._pipelineLayout != VK_NULL_HANDLE, "nullptr");
        return m_pipelineLayout._pipelineLayout;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanComputePipelineManager class
    * Multithreaded
    */
    class VulkanComputePipelineManager final
    {
    public:

        struct VulkanPipelineDesc
        {
            VulkanPipelineDesc() noexcept = default;

            explicit VulkanPipelineDesc(const ShaderProgram* program) noexcept
                : _program(program)
            {
            }

            const ShaderProgram* _program = nullptr;
        };

        explicit VulkanComputePipelineManager(VulkanDevice* device) noexcept;
        ~VulkanComputePipelineManager();

        [[nodiscard]] VulkanComputePipeline* acquireGraphicPipeline(const ComputePipelineState& state);

        bool removePipeline(VulkanComputePipeline* pipeline);
        void clear();

    private:

        VulkanComputePipelineManager(const VulkanComputePipelineManager&) = delete;
        VulkanComputePipelineManager& operator=(const VulkanComputePipelineManager&) = delete;

        VulkanDevice&    m_device;
        thread::Spinlock m_mutex;
        std::unordered_map<DescInfo<VulkanPipelineDesc>, VulkanComputePipeline*, DescInfo<VulkanPipelineDesc>::Hash, DescInfo<VulkanPipelineDesc>::Compare> m_pipelineComputeList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
