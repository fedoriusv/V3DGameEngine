#include "Memory.h"
#include "Common.h"
#include "FrameProfiler.h"

namespace v3d
{
namespace memory
{

std::string MemoryLabelName(MemoryLabel label)
{
    switch (label)
    {
#define STR(r) case ##r: return #r
        STR(MemoryDefault);
        STR(MemorySystem);
        STR(MemoryObject);
        STR(MemoryDynamic);
        STR(MemoryGame);
        STR(MemoryRenderCore);
        STR(MemoryCommandList);
#undef STR
    default:
        return "UNKNOWN";
    }
}

#if MEMORY_DEBUG
std::vector<std::tuple<void*, v3d::u64, v3d::memory::MemoryLabel, std::string, u32>> g_allocr;
std::recursive_mutex g_mutex;
#endif //MEMORY_DEBUG

void* internal_malloc(v3d::u64 size, MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line)
{
    void* ptr = malloc(size);

    TRACE_PROFILER_MEMORY_ALLOC(ptr, size, MemoryLabelName(label).c_str());

#if MEMORY_DEBUG
    std::lock_guard scope(g_mutex);
    g_allocr.push_back({ ptr, size, label, file, line });
#endif //MEMORY_DEBUG

    return ptr;
}

void internal_free(void* ptr, MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line)
{
    if (ptr)
    {
        TRACE_PROFILER_MEMORY_FREE(ptr, MemoryLabelName(label).c_str());

#if MEMORY_DEBUG
        std::lock_guard scope(g_mutex);
        auto found = std::find_if(g_allocr.begin(), g_allocr.end(), [ptr](const auto& q)->bool
            {
                return std::get<0>(q) == ptr;
            });

        ASSERT(found != g_allocr.end(), "not found");
        g_allocr.erase(found);
#endif //MEMORY_DEBUG

        free(ptr);
    }
}

void memory_test()
{
#if MEMORY_DEBUG
    std::lock_guard scope(g_mutex);
    ASSERT(g_allocr.empty(), "memory is not cleared");
#endif //MEMORY_DEBUG
}
} //namespace memory
} //namespace v3d


void* operator new(v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line) noexcept
{
    return v3d::memory::internal_malloc(size, label, align, file, line);
}

void* operator new[](v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line) noexcept
{
    return v3d::memory::internal_malloc(size, label, align, file, line);
}

void operator delete(void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line) noexcept
{
    v3d::memory::internal_free(ptr, label, align, file, line);
}

void operator delete[](void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line) noexcept
{
    v3d::memory::internal_free(ptr, label, align, file, line);
}
