#pragma once

#include "Common.h"
#include "Renderer/Core/Query.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DBuffer;
    class D3DGraphicsCommandList;
    class D3DHeapAllocator;
    class D3DRenderQueryManager;

    /**
    * D3DRenderQuery class. DirectX Render side
    */
    class D3DRenderQuery : public Query, public D3DResource
    {
    public:

        static D3D12_QUERY_TYPE convertQueryTypeToD3D(QueryType type) noexcept;

        D3DRenderQuery(ID3D12Device* device, QueryType type, u32 count, const QueryRespose& callback, const std::string& name = "", D3DHeapAllocator* allocator = nullptr) noexcept;
        ~D3DRenderQuery();

        bool create() override;
        void destroy() override;

    private:

        void invoke(QueryResult result, u32 size, const void* data);

#if D3D_DEBUG
        std::string m_debugName;
#endif

        friend D3DRenderQueryManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DQueryHeap class. DirectX Render side
    */
    class D3DQueryHeap : public D3DResource
    {
    public:

        D3D12_QUERY_HEAP_TYPE convertQueryTypeToHeapType(D3D12_QUERY_TYPE type) noexcept;

        explicit D3DQueryHeap(ID3D12Device* device, QueryType type, u32 count, D3DHeapAllocator* allocator) noexcept;
        ~D3DQueryHeap();

        bool create();
        void destroy();

        ID3D12QueryHeap* getHandle() const;
        D3D12_QUERY_TYPE getType() const;

    protected:

        ID3D12Device* const m_device;
        D3DHeapAllocator* const m_allocator;

        ID3D12QueryHeap* m_queryHeap;
        D3D12_QUERY_TYPE m_type;
        u32 m_count;
        D3DBuffer* m_resolveBuffer;

        friend D3DRenderQueryManager;
    };

    inline ID3D12QueryHeap* D3DQueryHeap::getHandle() const
    {
        ASSERT(m_queryHeap, "nullptr");
        return m_queryHeap;
    }

    inline D3D12_QUERY_TYPE D3DQueryHeap::getType() const
    {
        return m_type;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DRenderQueryManager final
    {
    public:

        explicit D3DRenderQueryManager(ID3D12Device* device, D3DHeapAllocator* allocator) noexcept;
        ~D3DRenderQueryManager();

        D3DQueryHeap* acquireQueryHeap(QueryType type);
        void resolve(D3DGraphicsCommandList* cmdList);
        void update();


        bool bind(D3DRenderQuery* query, D3DQueryHeap* heap, u32 start, u32 count);
        D3DQueryHeap* find(D3DRenderQuery* query) const;

        static const u32 s_queryHeapCount = 10000;

        struct Range
        {
            u32 start = 0;
            u32 count = 0;
        };

    private:

        ID3D12Device* const m_device;
        D3DHeapAllocator* const m_allocator;

        std::queue<D3DQueryHeap*> m_freeHeaps[toEnumType(QueryType::Count)];
        std::list<D3DQueryHeap*> m_usedHeaps[toEnumType(QueryType::Count)];
        D3DQueryHeap* m_currentHeap[toEnumType(QueryType::Count)];

        std::map<D3DRenderQuery*, std::tuple<D3DQueryHeap*, Range>> m_activeRenderQueries;
        std::map<D3DQueryHeap*, std::tuple<D3DRenderQuery*, u32>> m_resolvedQueries;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER