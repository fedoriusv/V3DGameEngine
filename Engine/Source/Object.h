#pragma once

#include "Common.h"
#include "Utils/RefCounted.h"

namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Object base game class. Game side
    */
    class Object : public utils::RefCounted
    {
    public:

        Object() = default;
        virtual ~Object() = default;

        void* operator new(size_t size) noexcept
        {
            return V3D_MALLOC(size, memory::MemoryLabel::MemoryObject);
        }
        
        void operator delete(void* ptr) noexcept
        {
            V3D_FREE(ptr, memory::MemoryLabel::MemoryObject);
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
