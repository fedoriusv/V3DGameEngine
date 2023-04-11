#include "MemoryManagement.h"
#include "MemoryPool.h"

namespace v3d
{
namespace memory
{

std::array<MemoryManagment::Allocation*, toEnumType(MemoryLabel::MemoryCount)> MemoryManagment::s_memoryAllocations =
    {
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

void* MemoryManagment::allocate(u32 size, MemoryLabel label, u32 align)
{
    void* ptr = s_memoryAllocations[toEnumType(label)]->alloc(size, align);
    ASSERT(ptr, "nullptr");

    return ptr;
}

void MemoryManagment::deallocate(void* ptr, MemoryLabel label, u32 align)
{
    ASSERT(ptr, "nullptr");
    s_memoryAllocations[toEnumType(label)]->dealloc(ptr, align);
}
} //namespace memory
} //namespace v3d
