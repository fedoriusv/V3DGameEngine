#pragma once

#include "Configuration.h"
#include <memory>
#include "Types.h"

namespace v3d
{
namespace memory
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * @brief MemoryLabel enum
    */
    enum class MemoryLabel
    {
        MemoryDefault = 0,
        MemorySystem,
        MemoryObject,
        MemoryDynamic,

        MemoryGame,
        MemoryRenderCore,
        MemoryCommandList,

        MemoryCount
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    void* internal_malloc(v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align = 0);
    void internal_free(void* ptr, v3d::memory::MemoryLabel label);

    template<class T>
    void internal_delete(T* ptr, v3d::memory::MemoryLabel label)
    {
        if (ptr)
        {
            ptr->~T();
            ::operator delete(const_cast<std::remove_const<T>::type*>(ptr), label, alignof(T));
        }
    }

} //namespace memory
} //namespace v3d

    /////////////////////////////////////////////////////////////////////////////////////////////////////

void* operator new(v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept;
void* operator new[](v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept;

void operator delete(void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept;
void operator delete[](void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align) noexcept;


    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if MEMORY_MANAGMENT
#   define V3D_NEW(type, label) new(label, alignof(type)) type
#   define V3D_NEW_ALIGNED(type, label, align) new(label, align) type
#   define V3D_DELETE(ptr, label) v3d::memory::internal_delete(ptr, label)

#   define V3D_MALLOC(size, label) v3d::memory::internal_malloc(size, label)
#   define V3D_MALLOC_ALIGNED(size, label, align) v3d::memory::internal_malloc(size, label, align)
#   define V3D_FREE(ptr, label) v3d::memory::internal_free(ptr, label)
#else
#   define V3D_NEW(type, label) new type
#   define V3D_NEW_ALIGNED(type, label, align) new type
#   define V3D_DELETE(ptr, label) delete ptr

#   define V3D_MALLOC(size, label) malloc(size)
#   define V3D_MALLOC_ALIGNED(size, label, align) malloc(size)
#   define V3D_FREE(ptr, label) free(ptr)
#endif //MEMORY_MANAGMENT

    /////////////////////////////////////////////////////////////////////////////////////////////////////