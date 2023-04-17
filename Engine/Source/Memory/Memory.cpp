#include "Memory.h"
#include "Common.h"

v3d::s32 g_allocation = 0;
std::vector<std::tuple<void*, v3d::u64, v3d::memory::MemoryLabel>> g_allocr;

namespace v3d
{
namespace memory
{
void* internal_malloc(v3d::u64 size, MemoryLabel label, v3d::u64 align)
{
    return malloc(size);
}

void internal_free(void* ptr, MemoryLabel label)
{
    free(ptr);
}

void memory_test()
{
    ASSERT(g_allocr.empty(), "memory is not cleared");
}
} //namespace memory
} //namespace v3d


void* operator new(v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept
{
    void* ptr = malloc(size);

    ++g_allocation;
    g_allocr.push_back({ ptr, size, label });

    return ptr;
}

void* operator new[](v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept
{
    ++g_allocation;
    return malloc(size);
}

void operator delete(void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept
{
    --g_allocation;
    auto found = std::find_if(g_allocr.begin(), g_allocr.end(), [ptr](std::tuple<void*, v3d::u64, v3d::memory::MemoryLabel>& q)->bool
        {
            return std::get<0>(q) == ptr;
        });

    ASSERT(found != g_allocr.end(), "not found");
    g_allocr.erase(found);

    free(ptr);
}

void operator delete[](void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept
{
    --g_allocation;
    free(ptr);
}
