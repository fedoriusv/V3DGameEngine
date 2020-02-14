#include "Common.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/Shader.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanDescriptorSet.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanGraphicPipeline;
    class VulkanDescriptorSetManager;
    class VulkanUniformBufferManager;
    class VulkanImage;
    class VulkanSampler;
    class VulkanUniformBuffer;
    class VulkanDescriptorPool;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanContextState class. Vulkan Render side
    */
    class VulkanContextState
    {
    public:

        explicit VulkanContextState(VkDevice device, VulkanDescriptorSetManager* descriptorSetManager, VulkanUniformBufferManager* unifromBufferManager) noexcept;

        void invalidateCommandBuffer(CommandTargetType type);

        bool isCurrentRenderPass(const VulkanRenderPass* pass) const;
        bool isCurrentFramebuffer(const VulkanFramebuffer* framebuffer) const;
        bool isCurrentPipeline(const VulkanGraphicPipeline* pipeline) const;

        bool setCurrentRenderPass(VulkanRenderPass* pass);
        bool setCurrentFramebuffer(VulkanFramebuffer* framebuffer);
        bool setCurrentFramebuffer(std::vector<VulkanFramebuffer*>& framebuffers);
        bool setCurrentPipeline(VulkanGraphicPipeline* pipeline);

        bool setCurrentVertexBuffers(const StreamBufferDescription& desc);
        void setClearValues(const VkRect2D& area, std::vector <VkClearValue>& clearValues);

        VulkanRenderPass *getCurrentRenderpass() const;
        VulkanFramebuffer* getCurrentFramebuffer() const;
        VulkanGraphicPipeline* getCurrentPipeline() const;

        const StreamBufferDescription& getStreamBufferDescription() const;

        bool setDynamicState(VkDynamicState state, const std::function<void()>& callback);
        void invokeDynamicStates(bool clear = true);

        bool prepareDescriptorSets(VulkanCommandBuffer* cmdBuffer, std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets);

        void bindTexture(const VulkanImage* image, const VulkanSampler* sampler, u32 arrayIndex, const Shader::SampledImage& info);
        void updateConstantBuffer(const Shader::UniformBuffer& info, u32 offset, u32 size, const void* data);

        void invalidateDescriptorSetsState();
        void updateDescriptorStates();

        std::vector<VkClearValue> m_renderPassClearValues;
        VkRect2D m_renderPassArea;

    private:

        struct BindingState
        {
            BindingState() = default;

            struct BindingData
            {
                struct BindingImageData
                {
                    const VulkanImage*      _image;
                    const VulkanSampler*    _sampler;
                };

                struct BindingBufferData
                {
                    const VulkanBuffer*     _buffer;
                    u64                     _offset;
                    u64                     _size;
                };

                std::variant<std::monostate, BindingImageData, BindingBufferData> _dataBinding;
            };

            std::array<std::tuple<BindingInfo, BindingData>, k_maxDescriptorBindingIndex> _set;

            u32  _activeBindingsFlags = 0;
            bool _dirtyFlag = false;

            void reset();

            void extractBufferOffsets(std::vector<u32>& offsets);

            bool isActiveBinding(u32 binding) const;
            bool isDirty() const;

            void bind(BindingType type, u32 binding, u32 arrayIndex, u32 layer, const VulkanImage* image, const VulkanSampler* sampler);
            void bind(BindingType type, u32 binding, u32 arrayIndex, const VulkanBuffer* buffer, u64 offset, u64 range);

            void apply(VulkanCommandBuffer* cmdBuffer, u64 frame, SetInfo& info);

            std::set<const VulkanResource*> _usedResources;
            std::vector<u32> _offsets;
        };

        void updateDescriptorSet(VulkanCommandBuffer* cmdBuffer, VkDescriptorSet set, const BindingState& info);

        static VkDescriptorBufferInfo makeVkDescriptorBufferInfo(const VulkanBuffer* buffer, u64 offset, u64 range);
        static VkDescriptorImageInfo makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler, VkImageLayout layout, s32 layer = -1);

        VkDevice m_device;

        //Current State
        std::pair<VulkanRenderPass*, bool>                  m_currentRenderpass;
        std::pair<std::vector<VulkanFramebuffer*>, bool>    m_currentFramebuffer;
        std::pair<VulkanGraphicPipeline*, bool>             m_currentPipeline;

        std::map<VkDynamicState, std::function<void()>>     m_stateCallbacks;

        std::pair<StreamBufferDescription, bool>            m_currentVertexBuffers;
        std::array<VkDescriptorSet, k_maxDescriptorSetIndex> m_currentDesctiptorsSets;

        VulkanDescriptorSetManager* m_descriptorSetManager;
        VulkanUniformBufferManager* m_unifromBufferManager;

        mutable BindingState m_currentBindingSlots[k_maxDescriptorSetIndex];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
