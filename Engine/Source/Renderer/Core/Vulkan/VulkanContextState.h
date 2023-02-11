#include "Common.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/Shader.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDeviceCaps.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorPool.h"
#include "VulkanQuery.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanGraphicPipeline;
    class VulkanDescriptorSetManager;
    class VulkanUniformBufferManager;
    class VulkanImage;
    class VulkanSampler;
    class VulkanUniformBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanContextState class. Vulkan Render side
    */
    class VulkanContextState
    {
    public:

        explicit VulkanContextState(VkDevice device, VulkanDescriptorSetManager* descriptorSetManager, VulkanUniformBufferManager* unifromBufferManager, VulkanQueryPoolManager* queryPoolManager) noexcept;

        void invalidateCommandBuffer(CommandTargetType type);

        bool isCurrentRenderPass(const VulkanRenderPass* pass) const;
        bool isCurrentFramebuffer(const VulkanFramebuffer* framebuffer) const;
        bool isCurrentPipeline(const Pipeline* pipeline) const;

        bool setCurrentRenderPass(VulkanRenderPass* pass);
        bool setCurrentFramebuffer(VulkanFramebuffer* framebuffer);
        bool setCurrentFramebuffer(std::vector<VulkanFramebuffer*>& framebuffers);
        bool setCurrentPipeline(Pipeline* pipeline);

        bool setCurrentVertexBuffers(const StreamBufferDescription& desc);
        void setClearValues(const VkRect2D& area, std::vector <VkClearValue>& clearValues);

        VulkanRenderPass *getCurrentRenderpass() const;
        VulkanFramebuffer* getCurrentFramebuffer() const;
        Pipeline* getCurrentPipeline() const;

        template<class Type>
        Type* getCurrentTypedPipeline() const
        {
            static_assert(std::is_same<Type, VulkanGraphicPipeline>() || std::is_same<Type, VulkanComputePipeline>(), "wrong type");
            return static_cast<Type*>(m_currentPipeline.first);
        }

        const StreamBufferDescription& getStreamBufferDescription() const;

        bool setDynamicState(VkDynamicState state, const std::function<void()>& callback);
        void invokeDynamicStates(bool clear = true);

        template<class Type>
        bool prepareDescriptorSets(VulkanCommandBuffer* cmdBuffer, std::vector<VkDescriptorSet>& sets, std::vector<u32>& offsets)
        {
            static_assert(std::is_same<Type, VulkanGraphicPipeline>() || std::is_same<Type, VulkanComputePipeline>(), "wrong type");
            if (VulkanContextState::getCurrentTypedPipeline<Type>()->getDescriptorSetLayouts()._setLayouts.empty())
            {
                return false;
            }

#if VULKAN_DEBUG
            if (!checkBindingsAndPipelineLayout())
            {
                ASSERT(false, "not same");
                return false;
            }
#endif
            sets.clear();
            offsets.clear();

            for (u32 setId = 0; setId < k_maxDescriptorSetIndex; ++setId)
            {
                BindingState& bindSet = m_currentBindingSlots[setId];
                if (bindSet.isDirty())
                {
                    SetInfo info;
                    bindSet.apply(cmdBuffer, 0, info);

                    VulkanDescriptorSetPool* pool = nullptr;
                    VkDescriptorSet set = m_descriptorSetManager->acquireDescriptorSet(VulkanDescriptorSetLayoutDescription(VulkanContextState::getCurrentTypedPipeline<Type>()->getPipelineLayoutDescription()._bindingsSet[setId]), info,
                        VulkanContextState::getCurrentTypedPipeline<Type>()->getDescriptorSetLayouts()._setLayouts[setId], pool);
                    if (set == VK_NULL_HANDLE)
                    {
                        ASSERT(false, "fail to allocate descriptor set");
                        return false;
                    }

                    ASSERT(pool, "nullptr");
                    //ASSERT(!pool->isCaptured(), "captured"); The pool can be captured, due few descriptor sets can lay in one pool
                    pool->captureInsideCommandBuffer(cmdBuffer, 0);

                    if (m_currentDesctiptorsSets[setId] != set)
                    {
                        m_currentDesctiptorsSets[setId] = set;
                        if (VulkanDeviceCaps::getInstance()->useDynamicUniforms)
                        {
                            auto updatedSet = m_updatedDescriptorsSets.insert(set);
                            if (updatedSet.second)
                            {
                                VulkanContextState::updateDescriptorSet(cmdBuffer, set, bindSet);
                            }
                        }
                        else
                        {
                            VulkanContextState::updateDescriptorSet(cmdBuffer, set, bindSet);
                        }
                    }

                    sets.push_back(set);

                    bindSet.extractBufferOffsets(offsets);
                    bindSet.reset();
                }
            }

            if (sets.empty())
            {
                return false;
            }

            return true;
        }

        void prepareRenderQueries(const std::function<VulkanCommandBuffer* ()>& cmdBufferGetter);

        void bindTexture(const VulkanImage* image, const VulkanSampler* sampler, u32 arrayIndex, const Shader::Image& info, const Image::Subresource& subresource);
        void bindTexture(const VulkanImage* image, u32 arrayIndex, const Shader::Image& info, const Image::Subresource& subresource);
        void bindSampler(const VulkanSampler* sampler, const Shader::Sampler& info);
        void bindStorageImage(const VulkanImage* image, u32 arrayIndex, const Shader::StorageImage& info, const Image::Subresource& subresource);
        void updateConstantBuffer(const Shader::UniformBuffer& info, u32 offset, u32 size, const void* data);

        const VulkanRenderQueryState* bindQuery(const VulkanQuery* query, u32 index, const std::string& tag);

        void invalidateDescriptorSetsState();
        void updateDescriptorStates();
        void invalidateRenderQueriesState();

        std::vector<VkClearValue> m_renderPassClearValues;
        VkRect2D m_renderPassArea;

    private:

        /**
        * @brief struct BindingState
        */
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

            std::vector<u32> _offsets;

            void reset();

            void extractBufferOffsets(std::vector<u32>& offsets);

            bool isActiveBinding(u32 binding) const;
            bool isDirty() const;

            void bind(BindingType type, u32 binding, u32 arrayIndex, const VulkanImage* image, const Image::Subresource& subresource, const VulkanSampler* sampler);
            void bind(BindingType type, u32 binding, u32 arrayIndex, const VulkanBuffer* buffer, u64 offset, u64 range);

            void apply(VulkanCommandBuffer* cmdBuffer, u64 frame, SetInfo& info);
        };

        void updateDescriptorSet(VulkanCommandBuffer* cmdBuffer, VkDescriptorSet set, const BindingState& info);

        static VkDescriptorBufferInfo makeVkDescriptorBufferInfo(const VulkanBuffer* buffer, u64 offset, u64 range);
        static VkDescriptorImageInfo makeVkDescriptorImageInfo(const VulkanImage* image, const VulkanSampler* sampler, VkImageLayout layout, const Image::Subresource& subresource);

        bool checkBindingsAndPipelineLayout();

        VkDevice m_device;

        //Current State
        std::pair<VulkanRenderPass*, bool>                  m_currentRenderpass;
        std::pair<std::vector<VulkanFramebuffer*>, bool>    m_currentFramebuffer;
        std::pair<Pipeline*, bool>                          m_currentPipeline;

        std::map<VkDynamicState, std::function<void()>>     m_stateCallbacks;

        std::pair<StreamBufferDescription, bool>            m_currentVertexBuffers;
        std::array<VkDescriptorSet, k_maxDescriptorSetIndex> m_currentDesctiptorsSets;
        std::set<VkDescriptorSet>                           m_updatedDescriptorsSets;
        std::map<const VulkanQuery*, VulkanRenderQueryState*> m_currentRenderQueryState;

        VulkanDescriptorSetManager* m_descriptorSetManager;
        VulkanUniformBufferManager* m_unifromBufferManager;
        VulkanQueryPoolManager* m_queryPoolManager;

        mutable BindingState m_currentBindingSlots[k_maxDescriptorSetIndex];
    };

    inline bool VulkanContextState::isCurrentRenderPass(const VulkanRenderPass* pass) const
    {
        return m_currentRenderpass.first == pass;
    }

    inline bool VulkanContextState::isCurrentPipeline(const Pipeline* pipeline) const
    {
        return m_currentPipeline.first == pipeline;
    }

    inline Pipeline* VulkanContextState::getCurrentPipeline() const
    {
        return m_currentPipeline.first;
    }

    inline VulkanRenderPass* VulkanContextState::getCurrentRenderpass() const
    {
        return m_currentRenderpass.first;
    }

    inline const StreamBufferDescription& VulkanContextState::getStreamBufferDescription() const
    {
        return m_currentVertexBuffers.first;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
