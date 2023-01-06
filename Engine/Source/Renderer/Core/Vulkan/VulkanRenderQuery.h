#pragma once

#include "Common.h"
#include "Renderer/Core/Query.h"
#include "Renderer/Core/RenderQueryPool.h"

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
    * @brief VulkanRenderQueryPool class. Vulkan render side
    */
    class VulkanRenderQueryPool final : public renderer::RenderQueryPool, public VulkanResource
    {
    public:

        static VkQueryType convertQueryTypeToVkQuery(QueryType type);

        explicit VulkanRenderQueryPool(VkDevice device, QueryType type, u32 size) noexcept;
        ~VulkanRenderQueryPool();

        bool create() override;
        void destroy() override;

        void reset() override;

        VkQueryPool getHandle() const;

    private:

        VkDevice m_device;
        VkQueryPool m_pool;
        bool m_waitComlete;

        friend class VulkanRenderQueryManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanRenderQueryManager class. Vulkan render side
    */
    class VulkanRenderQueryManager final
    {
    public:

        struct QueryState
        {
            const Query* const _query = nullptr;
            RenderQuery* const _renderQuery = nullptr;
            VulkanCommandBuffer* const _cmdBuffer = nullptr;
            std::string _tag;

            bool isValid() const
            {
                return _query != nullptr && _renderQuery != nullptr && _cmdBuffer != nullptr;
            }
        };

        VulkanRenderQueryManager() = delete;
        VulkanRenderQueryManager(const VulkanRenderQueryManager&) = delete;

        explicit VulkanRenderQueryManager(VkDevice device, u32 poolSize) noexcept;
        ~VulkanRenderQueryManager();

        RenderQuery* acquireRenderQuery(QueryType type, VulkanCommandBuffer* cmdBuffer);

        void updateRenderQuery();

        bool applyRenderQueryState(const QueryState& state);
        QueryState findRenderQueryState(Query* query) const;

    private:

        RenderQueryPool* acquireRenderQueryPool(QueryType type, VulkanCommandBuffer* cmdBuffer);
        void resetRenderQueryPool(RenderQueryPool* pool, VulkanCommandBuffer* cmdBuffer);

        VkDevice m_device;
        u32 m_poolSize;

        struct Pools
        {
            std::list<RenderQueryPool*> m_usedPools;
            std::list<RenderQueryPool*> m_freePools;
            RenderQueryPool* m_currentPool = nullptr;
        };
        Pools m_pools[toEnumType(QueryType::Count)];

        std::multimap<Query*, QueryState> m_renderStates;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
