#include "Common.h"
#include "Renderer/BufferProperties.h"
#include "Resource/Shader.h"

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

        inline bool isCurrentRenderPass(const VulkanRenderPass* pass) const;
        inline bool isCurrentFramebuffer(const VulkanFramebuffer* framebuffer) const;
        inline bool isCurrentPipeline(const VulkanGraphicPipeline* pipeline) const;

        bool setCurrentRenderPass(VulkanRenderPass* pass);
        bool setCurrentFramebuffer(VulkanFramebuffer* framebuffer);
        bool setCurrentPipeline(VulkanGraphicPipeline* pipeline);

        bool setCurrentVertexBuffers(StreamBufferDescription& desc);
        void setClearValues(const VkRect2D& area, std::vector <VkClearValue>& clearValues);

        VulkanRenderPass *getCurrentRenderpass() const;
        VulkanFramebuffer* getCurrentFramebuffer() const;
        VulkanGraphicPipeline* getCurrentPipeline() const;

        const StreamBufferDescription& getStreamBufferDescription() const;

        bool setDynamicState(VkDynamicState state, const std::function<void()>& callback);
        void invokeDynamicStates();

        bool acquireDescriptorSets(std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets);
        void updateDescriptorSet();

        void bindTexture(const VulkanImage* image, const VulkanSampler* sampler, u32 arrayIndex, const resource::Shader::SampledImage& reflaction);
        void updateConstantBuffer(u32 arrayIndex, const resource::Shader::UniformBuffer& reflaction, u32 offset, u32 size, const void* data);

        void invalidateDescriptorSetsState();

        std::vector<VkClearValue> m_renderPassClearValues;
        VkRect2D m_renderPassArea;

    private:

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

            union
            {
                BindingImageInfo  _imageBinding;
                BindingBufferInfo _bufferBinding;

            };
            //std::variant<BindingImageInfo, BindingBufferInfo> _descriptorBinding;

            u32                 _set        : 16;
            u32                 _binding    : 16;
            BindingType         _type       : 16;
            u32                 _arrayIndex : 16;
        };

        static VkDescriptorBufferInfo makeVkDescriptorBufferInfo(const VulkanBuffer* buffer, u64 offset, u64 range);
        static VkDescriptorImageInfo makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler);

        VkDevice m_device;

        std::pair<VulkanRenderPass*, bool>          m_currentRenderpass;
        std::pair <VulkanFramebuffer*, bool>        m_currentFramebuffer;
        std::pair <VulkanGraphicPipeline*, bool>    m_currentPipeline;

        std::map<VkDynamicState, std::function<void()>> m_stateCallbacks;

        std::pair<StreamBufferDescription, bool> m_currentVertexBuffers;

        VulkanDescriptorPool* m_currentPool;
        std::vector<VkDescriptorSet> m_currentSets;
        VulkanDescriptorSetManager* m_descriptorSetManager;

        VulkanUniformBufferManager* m_unifromBufferManager;

        void setBinding(BindingInfo& binding);

        std::vector<BindingInfo> m_updatedBindings;
        std::map<u32, BindingInfo> m_currentBindingCache;

        //std::array<std::vector<BindingInfo>, k_maxDescriptorSetIndex> m_descriptorSetsState;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
