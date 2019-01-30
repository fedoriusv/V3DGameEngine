#include "Common.h"
#include "Renderer/BufferProperties.h"
#include "Resource/Shader.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanCommandBufferManager.h"

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
    class VulkanImage;
    class VulkanSampler;
    class VulkanDescriptorPool;

    class VulkanContextState
    {
    public:

        explicit VulkanContextState(VkDevice device, VulkanDescriptorSetManager* descriptorSetManager) noexcept;

        void invalidateCommandBuffer(CommandTargetType type);

        inline bool isCurrentRenderPass(const VulkanRenderPass* pass) const;
        inline bool isCurrentFramebuffer(const VulkanFramebuffer* framebuffer) const;
        inline bool isCurrentPipeline(const VulkanGraphicPipeline* pipeline) const;

        bool setCurrentRenderPass(VulkanRenderPass* pass);
        bool setCurrentFramebuffer(VulkanFramebuffer* framebuffer);
        bool setCurrentPipeline(VulkanGraphicPipeline* pipeline);

        bool setCurrentVertexBuffers(StreamBufferDescription& desc);

        VulkanRenderPass *getCurrentRenderpass() const;
        VulkanFramebuffer* getCurrentFramebuffer() const;
        VulkanGraphicPipeline* getCurrentPipeline() const;

        const StreamBufferDescription& getStreamBufferDescription() const;

        bool setDynamicState(VkDynamicState state, const std::function<void()>& callback);
        void invokeDynamicStates();

        bool acquireDescriptorSets(std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets);
        void updateDescriptorSet();

        void bindTexture(VulkanImage* image, VulkanSampler* sampler, u32 arrayIndex, const resource::Shader::SampledImage& reflaction);
        void bindUnifrom();

    private:

        struct BindingInfo
        {
            BindingInfo();
            virtual ~BindingInfo();

            u32                 _set;
            u32                 _binding;
            VkDescriptorType    _type;
            u32                 _arrayIndex;

        };

        struct BindingImageInfo final : BindingInfo
        {
            BindingImageInfo();
            ~BindingImageInfo();

            VulkanImage* _image;
            VulkanSampler* _sampler;

        };

        struct BindingBufferInfo final : BindingInfo
        {
            BindingBufferInfo();
            ~BindingBufferInfo();

            VulkanBuffer* _buffer;
            u32 _offset;
            u32 _size;
        };

        VkDevice m_device;

        std::pair<VulkanRenderPass*, bool>          m_currentRenderpass;
        std::pair <VulkanFramebuffer*, bool>        m_currentFramebuffer;
        std::pair <VulkanGraphicPipeline*, bool>    m_currentPipeline;

        std::map<VkDynamicState, std::function<void()>> m_stateCallbacks;

        std::pair<StreamBufferDescription, bool> m_currentVertexBuffers;

        VulkanDescriptorSetManager* m_descriptorSetManager;
        VulkanDescriptorPool* m_currentPool;
        std::vector<VkDescriptorSet> m_currentSets;

        std::map<u32, std::map<u32, std::vector<BindingInfo*>>> m_bindingInfo;
        //void* _boundShaderStage[ShaderType::ShaderType_Count];
    };

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
