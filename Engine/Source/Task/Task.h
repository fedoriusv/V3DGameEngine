#pragma once

#include "Common.h"

namespace v3d
{
namespace task
{
    typedef void(*Fn_construct)(void*, void*);
    typedef void(*Fn_destruct)(void*);
    typedef void(*Fn_entryPoint)(void*);

    struct FunctionType
    {
        Fn_construct  _construct;
        Fn_destruct   _destruct;
        Fn_entryPoint _entryPoint;
    };

    constexpr u32 k_memoryBlockSize = 512;

    template<typename Func>
    FunctionType getFunctionType()
    {
        static_assert(sizeof(Func) <= k_memoryBlockSize, "Out of memory");
        return FunctionType
        {
            //Fn_construct
            [](void* mem, void* data) -> void
                {
                    new(mem) Func(std::forward<Func>(*static_cast<Func*>(data)));
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

    enum class Priority
    {
        Normal,
        High,
    };

    enum class Mask
    {
        AnyThread,
        MainThread,
        WorkerThread,
    };

    class Task
    {
    public:

        Task() noexcept;

        Priority        m_priority;
        Mask            m_mask;
        Fn_entryPoint   m_call;
        Fn_destruct     m_destruct;
        c8              m_memBlock[k_memoryBlockSize];
        std::string     m_name;
    };

} // namesapce task
} // namespace v3d