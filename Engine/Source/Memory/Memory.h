#pragma once

#include "Configuration.h"
#include <memory>
#include <assert.h>
#include "Types.h"


#if MEMORY_DEBUG
#   define __FILE_PATH__ __FILE__
#   define __FILE_LINE__ __LINE__
#else
#   define __FILE_PATH__ ""
#   define __FILE_LINE__ 0
#endif

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
        MemoryUI,

        MemoryCount
    };

    void memory_test();

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    void* internal_malloc(v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line);
    void internal_free(void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line);

    template<class T>
    void internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line)
    {
        if (ptr)
        {
            ptr->~T();

            if (std::is_polymorphic<T>::value)
            {
                uintptr_t* vtable_ptr = *(uintptr_t**)ptr;
                v3d::s64 offset = vtable_ptr[-2];
                //assert(offset == 0);
                //ptr += offset;
            }
            ::operator delete(const_cast<std::remove_const<T>::type*>(ptr), label, alignof(T), file, line);
        }
    }

} //namespace memory
} //namespace v3d

    /////////////////////////////////////////////////////////////////////////////////////////////////////

void* operator new(v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line) noexcept;
void* operator new[](v3d::u64 size, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line) noexcept;

void operator delete(void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line) noexcept;
void operator delete[](void* ptr, v3d::memory::MemoryLabel label, v3d::u64 align, const v3d::c8* file, v3d::u32 line) noexcept;


    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if MEMORY_MANAGMENT
#   define V3D_NEW(type, label) ::new(label, alignof(type), __FILE_PATH__, __FILE_LINE__) type
#   define V3D_NEW_ALIGNED(type, label, align) ::new(label, align, __FILE_PATH__, __FILE_LINE__) type
#   define V3D_PLACMENT_NEW(ptr, type) ::new(ptr) type
#   define V3D_DELETE(ptr, label) v3d::memory::internal_delete(ptr, label, __FILE_PATH__, __FILE_LINE__)

#   define V3D_MALLOC(size, label) v3d::memory::internal_malloc(size, label, 0, __FILE_PATH__, __FILE_LINE__)
#   define V3D_MALLOC_ALIGNED(size, label, align) v3d::memory::internal_malloc(size, label, align, __FILE_PATH__, __FILE_LINE__)
#   define V3D_FREE(ptr, label) v3d::memory::internal_free((void*)ptr, label, 0, __FILE_PATH__, __FILE_LINE__)
#else
#   define V3D_NEW(type, label) new type
#   define V3D_NEW_ALIGNED(type, label, align) new type
#   define V3D_REPLACMENT_NEW(ptr) ::new(ptr) type
#   define V3D_DELETE(ptr, label) delete ptr

#   define V3D_MALLOC(size, label) malloc(size)
#   define V3D_MALLOC_ALIGNED(size, label, align) malloc(size)
#   define V3D_FREE(ptr, label) free(ptr)
#endif //MEMORY_MANAGMENT

    /////////////////////////////////////////////////////////////////////////////////////////////////////