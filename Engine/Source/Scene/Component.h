#pragma once

#include "Common.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Component
    */
    class Component
    {
    public:

        Component() noexcept {};
        virtual ~Component() {};

        void* operator new(size_t size) noexcept
        {
            return malloc(size);
        }

        void operator delete(void* ptr) noexcept
        {
            free(ptr);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
