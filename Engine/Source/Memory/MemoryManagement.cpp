#include "MemoryManagement.h"
#include "MemoryPool.h"

namespace v3d
{
namespace memory
{
//    class SimpleAllocator final : public MemoryManagment::Memory
//    {
//    public:
//
//        SimpleAllocator() noexcept = default;
//        ~SimpleAllocator() = default;
//
//        void* alloc(u32 size) override
//        {
//            return malloc(size);
//        }
//
//        void dealloc(void* ptr) override
//        {
//            free(ptr);
//        }
//    };
//
//
//std::array<MemoryManagment::Memory*, toEnumType(MemoryManagment::Tag::MemoryCount)> MemoryManagment::s_memoryPools =
//    {
//        new SimpleAllocator(),
//        new SimpleAllocator(),
//        new SimpleAllocator(),
//        new SimpleAllocator(),
//    };
//
//void* MemoryManagment::allocate(u32 size, Tag tag)
//{
//    void* ptr = s_memoryPools[toEnumType(tag)]->alloc(size);
//    ASSERT(ptr, "nullptr");
//
//    return ptr;
//}
//
//void MemoryManagment::deallocate(void* ptr, Tag tag)
//{
//    ASSERT(ptr, "nullptr");
//    s_memoryPools[toEnumType(tag)]->dealloc(ptr);
//}
} //namespace memory
} //namespace v3d

void* operator new(v3d::u64 size, v3d::MemoryLabel label, v3d::u64 align)
{
    //TODO 
    return malloc(v3d::math::alignUp(size, align));
}

void* operator new[](v3d::u64 size, v3d::MemoryLabel label, v3d::u64 align)
{
    return nullptr;
}

void operator delete(void* ptr, v3d::MemoryLabel label)
{
    free(ptr);
}

void operator delete[](void* ptr, v3d::MemoryLabel label)
{
}
