#pragma once

#include "Common.h"
#include "TaskContainers.h"

namespace v3d
{
namespace task
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef void(*Fn_construct)(void*, void*);
    typedef void(*Fn_destruct)(void*);
    typedef void(*Fn_entryPoint)(void*);

    struct FunctionType
    {
        Fn_construct  _construct;
        Fn_destruct   _destruct;
        Fn_entryPoint _entryPoint;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Size of functor
    */
    constexpr u32 k_memoryBlockSize = 512;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Func>
    FunctionType getFunctionType()
    {
        static_assert(sizeof(Func) <= k_memoryBlockSize, "Out of memory");
        return FunctionType
        {
            //Fn_construct
            [](void* mem, void* data) -> void
                {
                    V3D_PLACMENT_NEW(mem, Func(std::forward<Func>(*static_cast<Func*>(data))));
                },
            //Fn_destruct
            [](void* func) -> void
                {
                    (static_cast<Func*>(func))->~Func();
                },
            //Fn_entryPoint
            [](void* func) -> void
                {
                    (*static_cast<Func*>(func))();
                }
        };
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TaskPriority enum
    */
    enum class TaskPriority
    {
        Normal,
        High,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Mask enum
    */
    enum class TaskMask
    {
        AnyThread,
        MainThread,
        WorkerThread,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum TaskStatus
    {
        Created,
        Queued,
        Executing,
        Completed,
        Deleted
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Task struct
    */
    struct Task : public ThreadSafeNode<Task>
    {
    public:

        Task() noexcept = default;

        TaskPriority             _priority;
        TaskMask                 _mask;
        Fn_entryPoint            _call;
        Fn_destruct              _destruct;
        c8                       _memBlock[k_memoryBlockSize];
        std::string              _name;

        //std::atomic<TaskStatus>  _result;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class TaskCounter
    {
    public:

        explicit TaskCounter(u32 value) noexcept;
        ~TaskCounter();

        TaskCounter(const TaskCounter&) = delete;
        TaskCounter& operator=(TaskCounter&) = delete;

        u32 wait();

        void increment();
        void decrement();

        bool compare_exchange(u32 prev, u32 next);

    private:
        std::atomic<u32>    m_counter;
        //TODO lock free
        std::mutex          m_mutex;
        std::vector<Task*>  m_list;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namesapce task
} // namespace v3d