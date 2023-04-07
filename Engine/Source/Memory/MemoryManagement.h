#pragma once

#include "Common.h"
#include "Memory.h"

namespace v3d
{
namespace memory
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    ///**
    //* @brief MemoryManagment
    //*/
    //class MemoryManagment
    //{
    //public:

    //    class Memory
    //    {
    //    public:

    //        Memory() noexcept = default;
    //        virtual ~Memory() = default;

    //        virtual void* alloc(u32 size) = 0;
    //        virtual void dealloc(void* ptr) = 0;
    //    };

    //    static void* allocate(u32 size, Tag tag);
    //    static void deallocate(void* ptr, Tag tag);

    //private:

    //    static std::array<Memory*, toEnumType(Tag::MemoryCount)> s_memoryPools;
    //};

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
