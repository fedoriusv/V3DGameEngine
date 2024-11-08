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

    class VulkanDevice;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDescriptorPool class. Vulkan Render side
    */
    class VulkanDescriptorSetPool : public VulkanResource
    {
    public:

        explicit VulkanDescriptorSetPool(VulkanDevice* device, VkDescriptorPoolCreateFlags flag, const std::string& name = "") noexcept;
        ~VulkanDescriptorSetPool();

        bool create(u32 setsCount, const std::vector<VkDescriptorPoolSize>& sizes);
        void destroy();

        bool reset();

        std::tuple<VkDescriptorSet, u32> getFreeDescriptorSet(VkDescriptorSetLayout layout);

    private:

        VulkanDescriptorSetPool(const VulkanDescriptorSetPool&) = delete;
        VulkanDescriptorSetPool& operator=(const VulkanDescriptorSetPool&) = delete;

        bool allocateDescriptorSets(VkDescriptorSetLayout layout, u32 count, std::vector<VkDescriptorSet>& descriptorSets);
        bool freeDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets);

        bool allocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet);
        bool freeDescriptorSet(VkDescriptorSet& descriptorSet);

        VulkanDevice&               m_device;
        VkDescriptorPool            m_pool;
        u32                         m_freeIndex;
        u32                         m_poolSize;
        VkDescriptorPoolCreateFlags m_flag;
#if VULKAN_DEBUG_MARKERS
        std::string                 m_debugName;
#endif
    };

    inline std::tuple<VkDescriptorSet, u32> VulkanDescriptorSetPool::getFreeDescriptorSet(VkDescriptorSetLayout layout)
    {
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
        if (m_freeIndex < m_poolSize)
        {
            u32 offset = m_freeIndex;
            ++m_freeIndex;
            if (VulkanDescriptorSetPool::allocateDescriptorSet(layout, descriptor))
            {
                return { descriptor, offset };
            }
        }

        return { VK_NULL_HANDLE, 0 };
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct GenericDescriptorPools
    {
        GenericDescriptorPools() noexcept = default;
        virtual ~GenericDescriptorPools() = default;

        virtual VulkanDescriptorSetPool* acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VulkanDevice* device, VkDescriptorPoolCreateFlags flag) = 0;
        virtual void destroyPools() = 0;

        virtual void updatePools() = 0;
        virtual void resetPools() = 0;
    };

    struct GlobalDescriptorPools : GenericDescriptorPools
    {
        GlobalDescriptorPools() noexcept = default;
        ~GlobalDescriptorPools();

        VulkanDescriptorSetPool* acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VulkanDevice* device, VkDescriptorPoolCreateFlags flag) override;
        void destroyPools() override;

        void updatePools() override;
        void resetPools() override;

    private:

        std::deque<VulkanDescriptorSetPool*> _freeDescriptorPools;
        std::deque<VulkanDescriptorSetPool*> _usedDescriptorPools;
        VulkanDescriptorSetPool*             _currentDescriptorPool = nullptr;

        static std::vector<VkDescriptorPoolSize> descriptorPoolSize(u32 countSets);
    };


    struct LayoutDescriptorPools : GenericDescriptorPools
    {
        LayoutDescriptorPools() noexcept = default;
        ~LayoutDescriptorPools();

        VulkanDescriptorSetPool* acquirePool(const VulkanDescriptorSetLayoutDescription& desc, VulkanDevice* device, VkDescriptorPoolCreateFlags flag) override;
        void destroyPools() override;

        void updatePools() override;
        void resetPools() override;

    private:

        VulkanDescriptorSetPool* createPool(const VulkanDescriptorSetLayoutDescription& desc, VulkanDevice* device, VkDescriptorPoolCreateFlags flag);

        struct LayoutPools
        {
            std::deque<VulkanDescriptorSetPool*> _usedList;
            std::deque<VulkanDescriptorSetPool*> _freeList;
            VulkanDescriptorSetPool* _currentPool = nullptr;
        };

        std::unordered_map<VulkanDescriptorSetLayoutDescriptionType, LayoutPools*, VulkanDescriptorSetLayoutDescriptionType::Hash, VulkanDescriptorSetLayoutDescriptionType::Compare> _pools;

        static const u32 s_maxSets = 4096;
        static const u32 s_multipliers = 32;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDescriptorSetProvider class. Vulkan Render side
    */
    class VulkanDescriptorPoolProvider final
    {
    public:

        explicit VulkanDescriptorPoolProvider(VulkanDevice* device, GenericDescriptorPools* strategy) noexcept;
        ~VulkanDescriptorPoolProvider();

        VulkanDescriptorSetPool* acquireDescriptorSetPool(const VulkanDescriptorSetLayoutDescription& desc, VkDescriptorPoolCreateFlags flag);
        void destroyDescriptorSetPools();

        void resetDescriptorSetPools();
        void updateDescriptorSetPools();

    private:

        VulkanDescriptorPoolProvider() = delete;
        VulkanDescriptorPoolProvider(const VulkanDescriptorPoolProvider&) = delete;

        VulkanDevice&                 m_device;
        GenericDescriptorPools* const m_descriptorPools;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
