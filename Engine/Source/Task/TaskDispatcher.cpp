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
    //TODO calc supported threads
    u32 supportedNumWorkingTreads = numWorkingThreads;
    m_numWorkingThreads = supportedNumWorkingTreads;

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
            freeTask(task);
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

Task* TaskDispatcher::prepareTask(TaskPriority priority, TaskMask mask, const FunctionType& funcType, void* funcData, const std::string& name)
{
    Task* task = V3D_NEW(Task, memory::MemoryLabel::MemorySystem);
    m_numCreatedTasks.fetch_add(1, std::memory_order_relaxed);

    task->_priority = priority;
    task->_mask = mask;
    task->_call = funcType._entryPoint;
    task->_destruct = funcType._destruct;
    task->_name = name;
    //task->_result = TaskStatus::Created;

    funcType._construct(task->_memBlock, funcData);

    return task;
}

void TaskDispatcher::pushTask(Task* task)
{
    TaskQueue* queue = nullptr;
    u32 threadID = TaskDispatcher::s_threadID;

    if (task->_priority == TaskPriority::High)
    {
        queue = m_taskQueue[TaskQueue::HighPriorityQueue];
    }
    else
    {
        switch (task->_mask)
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
    //task->_result = TaskStatus::Queued;
    if (m_numWatingTasks.load(std::memory_order_relaxed) > 0)
    {
        m_waitingCondition.notify_all();
    }
    queue->_mutex.unlock();
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
        task = priorityQueue->_tasks.front();
        priorityQueue->_tasks.pop();

        priorityQueue->_mutex.unlock();
        return task;

    }
    priorityQueue->_mutex.unlock();


    // Own thread list
    TaskQueue* threadQueue = m_taskQueue[TaskQueue::MainThreadQueue + threadID];
    threadQueue->_mutex.lock();
    if (!threadQueue->_tasks.empty())
    {
        task = threadQueue->_tasks.front();
        threadQueue->_tasks.pop();
    }
    threadQueue->_mutex.unlock();

    //TODO check another queues, steal from others queues

    return task;
}

void TaskDispatcher::run(Task* task)
{
    //task->_result = TaskStatus::Executing;
    task->_call(task->_memBlock);
    //task->_result = TaskStatus::Completed;
    task->_destruct(task->_memBlock);
}

void TaskDispatcher::freeTask(Task* task)
{
#if DEBUG
    task->_call = nullptr;
    task->_destruct = nullptr;
    memset(task->_memBlock, 0, k_memoryBlockSize);
    task->_name = "";
    //task->_result = TaskStatus::Deleted;
#endif // DEBUG

    V3D_DELETE(task, memory::MemoryLabel::MemorySystem);
    m_numCreatedTasks.fetch_sub(1, std::memory_order_relaxed);
}

bool TaskDispatcher::wait()
{
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

    return true;
}

} //namespace task
} //namespace v3d