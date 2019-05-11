#pragma once

#include "Common.h"
#include "Utils/RefCounted.h"

namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Object base game class. Game side
    */
    class Object : public utils::RefCounted
    {
    public:

        Object() = default;
        virtual ~Object() = default;

        void* operator new(size_t size) noexcept
        {
            return malloc(size);
        }
        
        void operator delete(void* ptr) noexcept
        {
            free(ptr);
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
