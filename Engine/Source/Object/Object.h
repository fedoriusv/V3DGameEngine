#pragma once

#include "Common.h"

namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Object //cloneable
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
