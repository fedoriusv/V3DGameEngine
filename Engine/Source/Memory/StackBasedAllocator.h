#pragma once

#include "Memory.h"

namespace v3d
{
namespace memory
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    class StackBasedAllocator
    {
    public:
        using value_type = T;

        StackBasedAllocator(void* buffer, size_t size) noexcept
            : m_memory(static_cast<char*>(buffer))
            , m_capacity(size)
            , m_offset(0)
        {
        }

        template<class U>
        StackBasedAllocator(const StackBasedAllocator<U>& other) noexcept
            : m_memory(other.m_memory)
            , m_capacity(other.m_capacity)
            , m_offset(other.m_offset)
        {
        }

        T* allocate(size_t n)
        {
            size_t bytes = n * sizeof(T);
            if (m_offset + bytes > m_capacity)
            {
                throw std::bad_alloc();
            }

            void* ptr = m_memory + m_offset;
            m_offset += bytes;
            return static_cast<T*>(ptr);
        }

        void deallocate(T*, size_t) noexcept
        {
        }

        template<class U>
        struct rebind
        { 
            using other = StackBasedAllocator<U>;
        };

        char* m_memory;
        size_t m_capacity;
        mutable size_t m_offset;
    };

    template<typename T>
    StackBasedAllocator<T> make_stack_allocator(u32 count)
    {
        u32 size = std::max<u32>(count * sizeof(T), 16U);
        void* buffer = alloca(size); // buffer lives until this function returns!
        return StackBasedAllocator<T>(buffer, size);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace memory
} //namespace v3d
