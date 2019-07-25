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

        void updateSwapchainIndex(u32 index);

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

        void bindTexture(const VulkanImage* image, const VulkanSampler* sampler, u32 arrayIndex, const Shader::SampledImage& reflaction);
        void updateConstantBuffer(u32 arrayIndex, const Shader::UniformBuffer& reflaction, u32 offset, u32 size, const void* data);

        void invalidateDescriptorSetsState();

        std::vector<VkClearValue> m_renderPassClearValues;
        VkRect2D m_renderPassArea;

    private:

        bool updateDescriptorSet(VulkanCommandBuffer* cmdBuffer, std::vector<VkDescriptorSet>& sets);

        enum BindingType
        {
            BindingType_Unknown = 0,
            BindingType_Uniform,
            BindingType_DynamicUniform,
            BindingType_Sampler,
            BindingType_Texture,
            BindingType_SamplerAndTexture,

            //TODO
        };

        struct BindingInfo
        {
            struct BindingImageInfo
            {
                VkDescriptorImageInfo _imageInfo;
                const VulkanImage*    _image;
                const VulkanSampler*  _sampler;
            };

            struct BindingBufferInfo
            {
                VkDescriptorBufferInfo      _bufferInfo;
                const VulkanBuffer*         _buffer;
                VulkanUniformBuffer*        _uniform;
            };

            BindingInfo();
            bool operator==(const BindingInfo& info) const;

            union //32
            {
                BindingImageInfo  _imageBinding; 
                BindingBufferInfo _bufferBinding;

            };
            //std::variant<BindingImageInfo, BindingBufferInfo> _descriptorBinding;

            u32                 _set        : 16; //6
            u32                 _binding    : 16; //6
            BindingType         _type       : 16; //4
            u32                 _arrayIndex : 16; //16
        };

        static VkDescriptorBufferInfo makeVkDescriptorBufferInfo(const VulkanBuffer* buffer, u64 offset, u64 range);
        static VkDescriptorImageInfo makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler, VkImageLayout layout, s32 layer = -1);

        VkDevice m_device;

        u32 m_swapchainIndex;

        std::pair<VulkanRenderPass*, bool>          m_currentRenderpass;
        std::pair <std::vector<VulkanFramebuffer*>, bool> m_currentFramebuffer;
        std::pair <VulkanGraphicPipeline*, bool>    m_currentPipeline;

        std::map<VkDynamicState, std::function<void()>> m_stateCallbacks;

        std::pair<StreamBufferDescription, bool> m_currentVertexBuffers;

        VulkanDescriptorSetManager* m_descriptorSetManager;
        VulkanUniformBufferManager* m_unifromBufferManager;
        
        VulkanDescriptorPool* m_currentPool;

        void setBinding(BindingInfo& binding);

        std::vector<BindingInfo> m_updatedBindings;
        std::map<u32, BindingInfo> m_currentBindingCache;


        std::vector<VkDescriptorSet> m_currentSets;

        //std::array<std::vector<BindingInfo>, k_maxDescriptorSetIndex> m_descriptorSetsState;

        class TransitionLayoutState
        {
        public:

            struct TransitionImageLayout
            {
                VulkanImage*            _image;
                VkImageSubresourceRange _subresource;
                VkPipelineStageFlags    _srcStage;
                VkPipelineStageFlags    _dstStage;
                VkImageLayout           _layout;

                bool operator==(const TransitionImageLayout& obj) const
                {
                    if (this == &obj)
                    {
                        return true;
                    }

                    if (_image != obj._image || memcmp(&_subresource, &obj._subresource, sizeof(VkImageSubresourceRange)))
                    {
                        return false;
                    }

                    return true;
                }

            };

            bool pushImageLayout(const TransitionImageLayout& obj);
            void flushImageLayout(VulkanCommandBuffer * cmdBuffer);

        private:

            struct ImageLayoutHash
            {
                size_t operator()(const TransitionImageLayout& obj) const
                {
                    //TODO
                    return 0;
                }
            };

            struct ImageLayoutEqual
            {
                bool operator()(const TransitionImageLayout& lhs, const TransitionImageLayout& rhs) const
                {
                    return lhs == rhs;
                }
            };

            std::unordered_set<TransitionImageLayout, ImageLayoutHash, ImageLayoutEqual> m_transitionImageLayouts;
        };

        TransitionLayoutState m_transitionLayoutState;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
