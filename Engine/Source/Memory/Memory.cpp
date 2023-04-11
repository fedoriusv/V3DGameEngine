#include "Memory.h"
#include "Common.h"

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
} //namespace memory
} //namespace v3d

void* operator new(v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept
{
    return malloc(v3d::math::alignUp(size, align));
}

void* operator new[](v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept
{
    return malloc(v3d::math::alignUp(size, align));
}

void operator delete(void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept
{
    free(ptr);
}

void operator delete[](void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept
{
    free(ptr);
}
