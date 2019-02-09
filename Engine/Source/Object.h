#pragma once

#include "Common.h"

namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Object base game class. Game side
    */
    class Object //cloneable, ref counter
    {
    public:

        Object() {};
        virtual ~Object() {};

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
