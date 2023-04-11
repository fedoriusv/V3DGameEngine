#pragma once

#include "Common.h"
#include "Memory.h"

namespace v3d
{
namespace memory
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief MemoryManagment
    */
    class MemoryManagment
    {
    public:

        class Allocation
        {
        public:

            Allocation() noexcept = default;
            virtual ~Allocation() = default;

            virtual void* alloc(u32 size, u32 align) = 0;
            virtual void dealloc(void* ptr, u32 align) = 0;
        };

        static void* allocate(u32 size, MemoryLabel label, u32 align);
        static void deallocate(void* ptr, MemoryLabel label, u32 align);

    private:

        static std::array<Allocation*, toEnumType(MemoryLabel::MemoryCount)> s_memoryAllocations;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
