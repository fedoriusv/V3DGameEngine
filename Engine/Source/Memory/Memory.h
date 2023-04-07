#pragma once

#include "Configuration.h"
//#include "Types.h"

/*
* @brief Internal memory managment
*/
#define MEMORY_MANAGMENT 0

namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * @brief MemoryLabel enum
    */
    enum class MemoryLabel
    {
        MemoryDefault = 0,
        MemoryGame,
        MemoryRenderCore,
        MemoryCommandList,

        MemoryCount
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d

    /////////////////////////////////////////////////////////////////////////////////////////////////////

void* operator new(v3d::u64 size, v3d::MemoryLabel label, v3d::u64 align);
void* operator new[](v3d::u64 size, v3d::MemoryLabel label, v3d::u64 align);

void operator delete(void* ptr, v3d::MemoryLabel label);
void operator delete[](void* ptr, v3d::MemoryLabel label);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if MEMORY_MANAGMENT
#   define V3D_NEW(type, label) new(label, alignof(type)) type
#   define V3D_NEW_ALIGNED(type, label, align) new(label, align) type

#   define V3D_DELETE(ptr, label) delete(ptr, label)
#else
#   define V3D_NEW(type, label) new type
#   define V3D_NEW_ALIGNED(type, label, align) new type

#   define V3D_DELETE(ptr, label) delete ptr; ptr = nullptr;
#endif //MEMORY_MANAGMENT

    /////////////////////////////////////////////////////////////////////////////////////////////////////