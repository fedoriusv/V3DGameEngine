#include "TaskScheduler.h"

#include "FrameProfiler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace task
{
thread_local u32 TaskDispatcher::s_threadID = 0;

TaskDispatcher::TaskDispatcher(u32 numWorkingThreads, DispatcherFlags flags) noexcept
    : m_numCreatedTasks(0)
    , m_numSleepingThreads(0)
    , m_flags(flags)
    , m_running(true)
{
    u32 numThreads = std::thread::hardware_concurrency();
    m_numWorkingThreads = std::clamp(numWorkingThreads, 1u, numThreads);

    u32 numQueue = TaskQueue::WorkerThreadQueue_0 + m_numWorkingThreads;
    m_taskQueue.reserve(numQueue);
    for (u32 index = 0; index < numQueue; ++index)
    {
        m_taskQueue.push_back(V3D_NEW(TaskQueue, memory::MemoryLabel::MemorySystem)());
    }

    m_workerThreads.reserve(m_numWorkingThreads);
    for (u32 index = 0; index < m_numWorkingThreads; ++index)
    {
        m_workerThreads.push_back(V3D_NEW(utils::Thread, memory::MemoryLabel::MemorySystem)());
        m_workerThreads[index]->run([this, index](void*) -> void
            {
                threadEntryPoint(index);
            }, nullptr);
    }
}

TaskDispatcher::~TaskDispatcher()
{
    m_running = false;
    m_waitingCondition.notify_all();
    for (u32 index = 0; index < m_numWorkingThreads; ++index)
    {
        utils::Thread* thread = m_workerThreads[index];
        thread->terminate();
        V3D_DELETE(thread, memory::MemoryLabel::MemorySystem);
    }
    m_workerThreads.clear();

    for (u32 index = 0; index < m_taskQueue.size(); ++index)
    {
        TaskQueue* queue = m_taskQueue[index];
        ASSERT(queue->_tasks.empty(), "must be empty");
        V3D_DELETE(queue, memory::MemoryLabel::MemorySystem);
    }
    m_taskQueue.clear();
}

void TaskDispatcher::workerThreadLoop()
{
    while (true)
    {
        Task* task = popTask();
        if (task)
        {
            run(task);
        }
        else if (wait()) //double check before sleep
        {
            if (!m_running) [[unlikely]]
            {
                return;
            }

            struct Locker
            {
                Locker(TaskDispatcher* disp) noexcept
                    : _dispatcher(disp)
                {
                }
                ~Locker() {}

                void lock() {}
                void unlock() {}

                TaskDispatcher* _dispatcher;
            };

            Locker lock(this);

            ++m_numSleepingThreads;
            m_waitingCondition.wait(lock, [this]() -> bool
                {
                    bool hasTask = false;
                    u32 threadID = TaskDispatcher::s_threadID;
                    m_taskQueue[TaskQueue::MainThreadQueue + threadID]->_mutex.lock();
                    hasTask = !m_taskQueue[TaskQueue::MainThreadQueue + threadID]->_tasks.empty();
                    m_taskQueue[TaskQueue::MainThreadQueue + threadID]->_mutex.unlock();

                    return hasTask || !m_running;
                });
            --m_numSleepingThreads;
        }
    }
}

void TaskDispatcher::lockThread()
{
    u32 threadID = TaskDispatcher::s_threadID;
    m_taskQueue[TaskQueue::MainThreadQueue + threadID]->_mutex.lock();
}

void TaskDispatcher::unlockThread()
{
    u32 threadID = TaskDispatcher::s_threadID;
    m_taskQueue[TaskQueue::MainThreadQueue + threadID]->_mutex.unlock();
}

u32 TaskDispatcher::getNumberOfWorkingThreads() const
{
    return m_numWorkingThreads + 1/*main thread*/;
}

u32 TaskDispatcher::currentWorkerThreadID()
{
    return TaskDispatcher::s_threadID;
}

void TaskDispatcher::threadEntryPoint(u32 threadID)
{
    [[maybe_unused]] std::string threadName = "WorkerThread_" + std::to_string(threadID);
#if DEBUG
    m_workerThreads[threadID]->setName(threadName);
#endif
#if TRACY_ENABLE
    tracy::SetThreadName(threadName.c_str());
#endif
    TaskDispatcher::s_threadID = threadID + 1;

    if (m_flags & WorkerThreadPerCore)
    {
        u32 mask = 1 << TaskDispatcher::s_threadID;
        m_workerThreads[threadID]->setAffinityMask(mask);
    }

    workerThreadLoop();
}

Task* TaskDispatcher::getTaskFromQueue(u32 id)
{
    Task* task = nullptr;

    ASSERT(id < m_taskQueue.size(), "ragen out");
    TaskQueue* queue = m_taskQueue[id];
    std::lock_guard lock(queue->_mutex);
    if (!queue->_tasks.empty())
    {
        u32 countTasks = queue->_tasks.size();
        while (countTasks)
        {
            Task* checkedTask = queue->_tasks.front();
            queue->_tasks.pop();
            --countTasks;

            if (checkedTask->m_cond && !checkedTask->m_cond())
            {
                queue->_tasks.push(checkedTask); //push in the end
                checkedTask->m_result.store(Task::Status::Waiting, std::memory_order_relaxed);
            }
            else
            {
                task = checkedTask;
                break;
            }
        }
    }

    return task;
}

void TaskDispatcher::pushTask(Task* task, TaskPriority priority, TaskMask mask)
{
    //update task. No need to lock
    task->m_priority = priority;
    task->m_mask = mask;

    TaskQueue* queue = nullptr;
    u32 threadID = TaskDispatcher::s_threadID;

    if (task->m_priority == TaskPriority::High)
    {
        queue = m_taskQueue[TaskQueue::HighPriorityQueue];
    }
    else
    {
        switch (task->m_mask)
        {
        case TaskMask::AnyThread:
            queue = m_taskQueue[TaskQueue::MainThreadQueue + threadID];
            break;

        case TaskMask::MainThread:
            queue = m_taskQueue[TaskQueue::MainThreadQueue];
            break;

        case TaskMask::WorkerThread:
        {
            u32 starWorkerThread = TaskQueue::WorkerThreadQueue_0;
            if (threadID > 0) //caller thread
            {
                queue = m_taskQueue[starWorkerThread + threadID];
            }
            else
            {
                u32 queueID = m_roundThreadCounter.fetch_add(1, std::memory_order_relaxed) % m_numWorkingThreads;
                queue = m_taskQueue[starWorkerThread + queueID];
            }
            break;
        }

        default:
            ASSERT(false, "is not handled");
        }
    }
    ASSERT(queue, "must be selected");

    {
        std::lock_guard lock(queue->_mutex);
        queue->_tasks.push(task);
        task->m_result.store(Task::Status::Scheduled, std::memory_order_relaxed);
    }

    if (m_numSleepingThreads > 0)
    {
        m_waitingCondition.notify_all();
    }
}

Task* TaskDispatcher::popTask()
{
    Task* task = nullptr;
    u32 threadID = TaskDispatcher::s_threadID;

    // Check priority list first
    task = TaskDispatcher::getTaskFromQueue(TaskQueue::HighPriorityQueue);
    if (task)
    {
        return task;
    }

    // Own thread list
    task = TaskDispatcher::getTaskFromQueue(TaskQueue::MainThreadQueue + threadID);
    if (task)
    {
        return task;
    }


    if (threadID == 0 && !(m_flags & DispatcherFlag::AllowToMainThreadStealTasks))
    {
        ASSERT(utils::Thread::getCurrentThread() == utils::Thread::getMainThreadId(), "main thread");
        return nullptr;
    }

    //Steal from other queues
    for (s32 index = 1; index < m_numWorkingThreads; ++index)
    {
        u32 threadJ = threadID + index;
        threadJ = threadJ % m_numWorkingThreads + 1;

        task = TaskDispatcher::getTaskFromQueue(TaskQueue::MainThreadQueue + threadJ);
        if (task)
        {
            return task;
        }
    }

    return task;
}

void TaskDispatcher::run(Task* task)
{
    task->m_result.store(Task::Status::Executing, std::memory_order_relaxed);
    task->m_func();

    {
        std::lock_guard<std::mutex> lock(task->m_mutex);
        task->m_result.store(Task::Status::Completed, std::memory_order_relaxed);
    }
    task->m_wait.notify_all();
}

bool TaskDispatcher::wait()
{
    u32 threadID = TaskDispatcher::s_threadID;

    for (s32 index = 0; index < m_taskQueue.size(); ++index)
    {
        std::lock_guard lock(m_taskQueue[index]->_mutex);
        if (!m_taskQueue[index]->_tasks.empty())
        {
            return false;
        }
    }

    return true;
}

} //namespace task
} //namespace v3d