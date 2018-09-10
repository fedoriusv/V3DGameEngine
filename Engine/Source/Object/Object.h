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
            return nullptr;
        }
        
        void operator delete(void* ptr) noexcept
        {
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
