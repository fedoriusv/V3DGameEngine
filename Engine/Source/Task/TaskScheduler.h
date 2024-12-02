#pragma once

#include "Common.h"
#include "Utils/Thread.h"
#include "Task.h"

namespace v3d
{
namespace task
{
    struct CPUInfo
    {
        u32 _physicalCPUs;
        u32 _logicalCPUs;
    };

    class Dispatcher
    {
    public:

        struct TaskList
        {
            TaskList()
                : _tasks()
                , _mutex()
            {
            }
            //free lock cont
            std::vector<Task*>   _tasks;
            std::mutex           _mutex;
        };

        struct TaskQueue
        {
            enum Type
            {
                HighPriorityQueue,
                RezervedQueue,
                MainThreadQueue,
                WorkerThreadQueue_0
            };

            std::mutex          _mutex;
            std::queue<Task*>   _tasks;

        };

        struct Locker
        {
            Locker(Dispatcher* disp) noexcept
                : _dispatcher(disp)
            {
            }

            ~Locker()
            {
                unlock();
            }

            void lock()
            {
                _dispatcher->lockQueues();
            }

            void unlock()
            {
                _dispatcher->unlockQueues();
            }

            Dispatcher* _dispatcher;
        };

        Dispatcher(u32 numWorkingThreads) noexcept;
        ~Dispatcher();

        Task* prepareTask(Priority priority, Mask mask, const FunctionType& funcType, void* funcData);

        void pushTask(Task* task);
        Task* popTask();

        void run(Task* task);
        bool wait();
        void freeTask(Task* task);

        void lockQueues();
        void unlockQueues();

        void workerThreadLoop();

    private:

        void threadEntryPoint(u32 threadID);

        u32 m_numWorkingThreads;

        //TODO lock free
        std::vector<Task*>          m_freeTaskList;
        std::mutex                  m_freeTaskMutex;

        std::vector<TaskQueue*>     m_taskQueue;
        std::vector<utils::Thread*> m_worker_threads;
        std::atomic<u32>            m_numCreatedTasks;
        std::atomic<u32>            m_numWatingTasks;
        std::condition_variable_any m_waitingCondition;

        std::atomic<u64>            m_roundThreadCounter;

        thread_local static u32 s_threadID;
    };


    class TaskScheduler
    {
    public:

        explicit TaskScheduler(u32 numWorkerThreads) noexcept
            : m_dispatcher(numWorkerThreads)
        {
        }

        template<typename Func, typename...Args>
        void spawnTask(Priority priority, Mask mask, Func&& func, Args&&... args);

        void mainThreadLoop()
        {
            ASSERT(utils::Thread::getCurrentThread() == utils::Thread::getMainThreadId(), "must be main thread");
            Task* task = m_dispatcher.popTask();
            if (task)
            {
                m_dispatcher.run(task);
                m_dispatcher.freeTask(task);
            }
        }

    private:

        Dispatcher m_dispatcher;
    };

    template<typename Func, typename ...Args>
    inline void TaskScheduler::spawnTask(Priority priority, Mask mask, Func&& func, Args && ...args)
    {
        auto fn = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
        Task* task = m_dispatcher.prepareTask(priority, mask, getFunctionType<decltype(fn)>(), &fn);

        m_dispatcher.pushTask(task);
    }

} // namesapce task
} // namespace v3d

