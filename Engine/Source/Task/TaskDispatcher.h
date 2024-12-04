#pragma once

#include "Common.h"
#include "Utils/Thread.h"
#include "Task.h"

namespace v3d
{
namespace task
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct CPUInfo
    {
        u32 _physicalCPUs;
        u32 _logicalCPUs;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Dispatcher class
    */
    class TaskDispatcher
    {
    public:

        enum DispatcherFlag
        {
            WorkerThreadPerCore
        };

        typedef u32 DispatcherFlags;

        struct TaskQueue
        {
            enum Type
            {
                HighPriorityQueue,
                MainThreadQueue,
                WorkerThreadQueue_0
            };

            std::mutex          _mutex;
            std::queue<Task*>   _tasks;
        };

        TaskDispatcher(u32 numWorkingThreads, DispatcherFlags flags) noexcept;
        ~TaskDispatcher();

        Task* prepareTask(TaskPriority priority, TaskMask mask, const FunctionType& funcType, void* funcData, const std::string& name = "");

        void pushTask(Task* task);
        Task* popTask();

        void run(Task* task);
        bool wait();
        void freeTask(Task* task);

        void workerThreadLoop();

    private:

        void threadEntryPoint(u32 threadID);

        u32 m_numWorkingThreads;

        std::vector<TaskQueue*>     m_taskQueue;
        std::vector<utils::Thread*> m_worker_threads;
        std::atomic<u32>            m_numCreatedTasks;
        std::atomic<u32>            m_numWatingTasks;
        std::condition_variable_any m_waitingCondition;

        std::atomic<u64>            m_roundThreadCounter;
        DispatcherFlags             m_flags;

        thread_local static u32     s_threadID;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namesapce task
} // namespace v3d

