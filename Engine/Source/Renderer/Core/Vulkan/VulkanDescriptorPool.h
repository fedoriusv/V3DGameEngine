#pragma once

#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanResource.h"
#include "VulkanDescriptorSet.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDescriptorPool class. Vulkan Render side
    */
    class VulkanDescriptorSetPool : public VulkanResource
    {
    public:

        VulkanDescriptorSetPool(VkDevice device, VkDescriptorPoolCreateFlags flag) noexcept;
        ~VulkanDescriptorSetPool();

        bool create(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes);
        void destroy();

        bool reset();
        u64 getCountDescriptorSets() const;

        VkDescriptorSet createDescriptorSet(const SetInfo& info, VkDescriptorSetLayout layout);
        VkDescriptorSet getDescriptorSet(const SetInfo& info);

    private:

        VulkanDescriptorSetPool() = delete;
        VulkanDescriptorSetPool(const VulkanDescriptorSetPool&) = delete;

        bool createDescriptorPool(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes);

        bool allocateDescriptorSets(std::vector<VkDescriptorSetLayout>& layout, std::vector<VkDescriptorSet>& descriptorSets);
        bool freeDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets);

        bool allocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet);
        bool freeDescriptorSet(VkDescriptorSet& descriptorSet);

        VkDevice m_device;

        VkDescriptorPoolCreateFlags m_flag;
        VkDescriptorPool m_pool;

        std::unordered_map<SetInfo, VkDescriptorSet, SetInfo::Hash, SetInfo::Equal> m_descriptorSets;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct GenericDescriptorPools
    {
        GenericDescriptorPools() noexcept = default;
        virtual ~GenericDescriptorPools() = default;

        virtual VulkanDescriptorSetPool* acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VkDevice device, VkDescriptorPoolCreateFlags flag) = 0;
        virtual void destroyPools() = 0;

        virtual void updatePools() = 0;
        virtual void resetPools() = 0;
    };

    struct GlobalDescriptorPools : GenericDescriptorPools
    {
        GlobalDescriptorPools() noexcept;
        ~GlobalDescriptorPools();

        VulkanDescriptorSetPool* acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VkDevice device, VkDescriptorPoolCreateFlags flag) override;
        void destroyPools() override;

        void updatePools() override;
        void resetPools() override;

    private:

        std::deque<VulkanDescriptorSetPool*> _freeDescriptorPools;
        std::deque<VulkanDescriptorSetPool*> _usedDescriptorPools;

        static std::vector<VkDescriptorPoolSize> s_poolSizes;
        static const u32 s_maxSets = 256;
    };


    struct LayoutDescriptorPools : GenericDescriptorPools
    {
        LayoutDescriptorPools() noexcept;
        ~LayoutDescriptorPools();

        VulkanDescriptorSetPool* acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VkDevice device, VkDescriptorPoolCreateFlags flag) override;
        void destroyPools() override;

        void updatePools() override;
        void resetPools() override;

    private:

        VulkanDescriptorSetPool* createPool(const VulkanDescriptorSetLayoutDescription& desc, VkDevice device, VkDescriptorPoolCreateFlags flag);

        struct LayoutPools
        {
            std::deque<VulkanDescriptorSetPool*> _usedList;
            std::deque<VulkanDescriptorSetPool*> _freeList;
        };

        std::unordered_map<VulkanDescriptorSetLayoutDescription, LayoutPools*, VulkanDescriptorSetLayoutDescription::Hash, VulkanDescriptorSetLayoutDescription::Equal> _pools;

        static const u32 s_maxSets = 256;
        static const u32 s_multipliers = 32;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDescriptorSetProvider class. Vulkan Render side
    */
    class VulkanDescriptorPoolProvider
    {
    public:

        VulkanDescriptorPoolProvider(VkDevice device, GenericDescriptorPools* strategy) noexcept;
        ~VulkanDescriptorPoolProvider();

        VulkanDescriptorSetPool* acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VkDescriptorPoolCreateFlags flag);
        void destroyPools();

        void updatePools();
        void resetPools();

    private:

        VulkanDescriptorPoolProvider() = delete;
        VulkanDescriptorPoolProvider(const VulkanDescriptorPoolProvider&) = delete;

        VkDevice m_device;
        GenericDescriptorPools* const m_descriptorPools;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
