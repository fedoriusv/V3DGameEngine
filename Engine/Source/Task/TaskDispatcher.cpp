#include "TaskScheduler.h"

namespace v3d
{
namespace task
{
thread_local u32 TaskDispatcher::s_threadID = 0;

TaskDispatcher::TaskDispatcher(u32 numWorkingThreads, DispatcherFlags flags) noexcept
    : m_numCreatedTasks(0)
    , m_numWatingTasks(0)
    , m_flags(flags)
{
    u32 nthreads = std::thread::hardware_concurrency();
    m_numWorkingThreads = std::clamp(numWorkingThreads, 1u, nthreads);

    u32 numQueue = TaskQueue::WorkerThreadQueue_0 + m_numWorkingThreads;
    m_taskQueue.reserve(numQueue);
    for (u32 index = 0; index < numQueue; ++index)
    {
        m_taskQueue.push_back(new TaskQueue());
    }

    m_worker_threads.reserve(m_numWorkingThreads);
    for (u32 index = 0; index < m_numWorkingThreads; ++index)
    {
        m_worker_threads.push_back(new utils::Thread());
        m_worker_threads[index]->run([this, index](void*) -> void
            {
                threadEntryPoint(index);
            }, nullptr);
    }
}

TaskDispatcher::~TaskDispatcher()
{
    //TODO
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

            ++m_numWatingTasks;
            m_waitingCondition.wait(lock);
            --m_numWatingTasks;
        }
    }
}

void TaskDispatcher::threadEntryPoint(u32 threadID)
{
#if DEBUG
    std::string threadName = "WorkerThread_" + std::to_string(threadID);
    m_worker_threads[threadID]->setName(threadName);
#endif
    TaskDispatcher::s_threadID = threadID + 1;

    if (m_flags & WorkerThreadPerCore)
    {
        u32 mask = 1 << TaskDispatcher::s_threadID;
        m_worker_threads[threadID]->setAffinityMask(mask);
    }

    workerThreadLoop();
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

    queue->_mutex.lock();
    queue->_tasks.push(task);
    if (m_numWatingTasks.load(std::memory_order_relaxed) > 0)
    {
        m_waitingCondition.notify_all();
    }
    queue->_mutex.unlock();
    task->m_result.store(Task::Status::Scheduled, std::memory_order_relaxed);
}

Task* TaskDispatcher::popTask()
{
    Task* task = nullptr;
    u32 threadID = TaskDispatcher::s_threadID;

    // Check priority list first
    TaskQueue* priorityQueue = m_taskQueue[TaskQueue::HighPriorityQueue];
    priorityQueue->_mutex.lock();
    if (!priorityQueue->_tasks.empty())
    {
        u32 countTasks = priorityQueue->_tasks.size();
        while (countTasks)
        {
            Task* checkedTask = priorityQueue->_tasks.front();
            priorityQueue->_tasks.pop();
            --countTasks;

            if (checkedTask->m_cond && !checkedTask->m_cond())
            {
                priorityQueue->_tasks.push(checkedTask); //push in the end
                checkedTask->m_result.store(Task::Status::Waiting, std::memory_order_relaxed);
            }
            else
            {
                task = checkedTask;
                break;
            }
        }
    }
    priorityQueue->_mutex.unlock();


    // Own thread list
    TaskQueue* threadQueue = m_taskQueue[TaskQueue::MainThreadQueue + threadID];
    threadQueue->_mutex.lock();
    if (!threadQueue->_tasks.empty())
    {
        u32 countTasks = threadQueue->_tasks.size();
        while (countTasks)
        {
            Task* checkedTask = threadQueue->_tasks.front();
            threadQueue->_tasks.pop();
            --countTasks;

            if (checkedTask->m_cond && !checkedTask->m_cond())
            {
                threadQueue->_tasks.push(checkedTask); //push in the end
                checkedTask->m_result.store(Task::Status::Waiting, std::memory_order_relaxed);
            }
            else
            {
                task = checkedTask;
                break;
            }
        }
    }
    threadQueue->_mutex.unlock();

    //TODO check another queues, steal from others queues

    return task;
}

void TaskDispatcher::run(Task* task)
{
    task->m_result.store(Task::Status::Executing, std::memory_order_relaxed);
    task->m_func();
    task->m_result.store(Task::Status::Completed, std::memory_order_relaxed);
    task->m_wait.notify_all();
}

bool TaskDispatcher::wait()
{
    bool sleep = true;
    u32 threadID = TaskDispatcher::s_threadID;

    TaskQueue* threadQueue = m_taskQueue[TaskQueue::MainThreadQueue + threadID];
    if (!threadQueue->_mutex.try_lock())
    {
        return false;
    }

    sleep = threadQueue->_tasks.empty();
    threadQueue->_mutex.unlock();

    /*for (s32 index = 0; index < m_taskQueue.size(); ++index)
    {
        if (!m_taskQueue[index]->_mutex.try_lock())
        {
            //unlock all
            --index;
            for (; index >= 0; --index)
            {
                m_taskQueue[index]->_mutex.unlock();
            }

            return false;
        }

        if (!m_taskQueue[index]->_tasks.empty())
        {
            for (; index >= 0; --index)
            {
                m_taskQueue[index]->_mutex.unlock();
            }

            return false;
        }
    }*/

    return sleep;
}

} //namespace task
} //namespace v3d