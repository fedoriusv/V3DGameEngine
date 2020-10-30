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
            //TODO use memory pool
            return malloc(size);
        }
        
        void operator delete(void* ptr) noexcept
        {
            //TODO use memory pool
            free(ptr);
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
