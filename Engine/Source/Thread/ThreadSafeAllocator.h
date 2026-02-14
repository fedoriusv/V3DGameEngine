#pragma once

#include "Configuration.h"
#include "Common.h"
#include "Memory.h"
#include "Task/TaskDispatcher.h"
#include "Thread/Spinlock.h"

namespace v3d
{
namespace thread
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
    * @brief ThreadSafeAllocator
    * Allocate memory pool per worker. Worker ID can be extracted from utils::Thread.
    * Memory should be reseted and reused
    */
    class ThreadSafeAllocator
    {
    public:

        struct Allocation
        {
            void* _ptr = nullptr;
            u64   _size;

            operator bool() const
            {
                return _ptr != nullptr;
            }

            c8* begin()
            {
                return reinterpret_cast<c8*>(_ptr);
            }

            c8* end()
            {
                return reinterpret_cast<c8*>(_ptr) + _size;
            }
        };

        ThreadSafeAllocator(u64 size, u32 workers);
        ~ThreadSafeAllocator();

        Allocation allocate(u64 size, u64 alignment = k_defaultAlignment, u32 workerID = task::TaskDispatcher::currentWorkerThreadID());
        void reset(u32 workerID = 0);

        template<typename T, typename ...Args>
        T* construct(Args&&... args)
        {
            return new(allocate(sizeof(T), alignof(T))._ptr) T(std::forward<Args>(args)...);
        }

    private:

        struct WorkerPool
        {
            WorkerPool()
                : _memory(nullptr)
                , _offest(0)
            {
            }

            WorkerPool(const WorkerPool& pool)
                : _memory(pool._memory)
                , _offest(pool._offest.load(std::memory_order_relaxed))
            {
            }

            void* _memory;
            std::atomic<u32> _offest;

        };
        std::vector<WorkerPool> m_workerPools;
        u64 m_poolSize;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace thread
} //namespace v3d

