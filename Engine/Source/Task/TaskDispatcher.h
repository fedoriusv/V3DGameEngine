#pragma once

#include "Common.h"
#include "Thread/Thread.h"
#include "Task.h"

namespace v3d
{
namespace task
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Dispatcher class
    */
    class TaskDispatcher
    {
    public:

        enum DispatcherFlag
        {
            WorkerThreadPerCore,
            AllowToMainThreadStealTasks
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

        void pushTask(Task* task, TaskPriority priority, TaskMask mask);
        Task* popTask();

        void run(Task* task);
        bool wait();

        void workerThreadLoop();

        void lockThread();
        void unlockThread();

        u32 getNumberOfWorkingThreads() const;
        static u32 currentWorkerThreadID();

    private:

        void threadEntryPoint(u32 threadID);

        Task* getTaskFromQueue(u32 id);

        u32                          m_numWorkingThreads;

        std::vector<TaskQueue*>      m_taskQueue;
        std::vector<thread::Thread*> m_workerThreads;
        std::atomic<u32>             m_numCreatedTasks;
        std::atomic<u32>             m_numSleepingThreads;
        std::condition_variable_any  m_waitingCondition;

        std::atomic<u64>             m_roundThreadCounter;
        DispatcherFlags              m_flags;
        bool                         m_running;

        thread_local static u32      s_threadID;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namesapce task
} // namespace v3d

