#include "TaskScheduler.h"

namespace v3d
{
namespace task
{
    thread_local u32 Dispatcher::s_threadID = 0;

    Dispatcher::Dispatcher(u32 numWorkingThreads) noexcept
        : m_numCreatedTasks(0)
        , m_numWatingTasks(0)
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

        //m_worker_threads.reserve(m_numWorkingThreads);
        //for (u32 index = 0; index < m_numWorkingThreads; ++index)
        //{
        //    m_worker_threads.emplace_back([this, index]() -> void
        //        {
        //            threadEntryPoint(index);
        //        });
        //}
    }

    Dispatcher::~Dispatcher()
    {

    }

    void Dispatcher::workerThreadLoop()
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
                Dispatcher::Locker lock(this);

                ++m_numWatingTasks;
                m_waitingCondition.wait(lock);
                //std::this_thread::yield();
                --m_numWatingTasks;
            }
        }
    }

    void Dispatcher::threadEntryPoint(u32 threadID)
    {
#if DEBUG
        std::string threadName = "WorkerThread_" + std::to_string(threadID);
        m_worker_threads[threadID]->setName(threadName);
#endif
        Dispatcher::s_threadID = threadID + 1;

        //u32 mask = 1 << Dispatcher::s_threadID;
        //m_worker_threads[threadID]->setAffinityMask(mask);

        workerThreadLoop();
    }

    Task* Dispatcher::prepareTask(Priority priority, Mask mask, const FunctionType& funcType, void* funcData)
    {
        Task* task = nullptr;

        m_freeTaskMutex.lock();
        if (!m_freeTaskList.empty())
        {
            task = m_freeTaskList.back();
            m_freeTaskList.pop_back();
        }
        m_freeTaskMutex.unlock();

        if (!task)
        {
            task = new Task();
            m_numCreatedTasks.fetch_add(1, std::memory_order_relaxed);
        }

        task->m_priority = priority;
        task->m_mask = mask;
        task->m_call = funcType._entryPoint;
        task->m_destruct = funcType._destruct;

        funcType._construct(task->m_memBlock, funcData);

        return task;
    }

    void Dispatcher::pushTask(Task* task)
    {
        TaskQueue* queue = nullptr;
        u32 threadID = Dispatcher::s_threadID;

        if (task->m_priority == Priority::High)
        {
            queue = m_taskQueue[TaskQueue::HighPriorityQueue];
        }
        else
        {
            switch (task->m_mask)
            {
            case Mask::AnyThread:
                queue = m_taskQueue[TaskQueue::MainThreadQueue + threadID];
                break;

            case Mask::MainThread:
                queue = m_taskQueue[TaskQueue::MainThreadQueue];
                break;

            case Mask::WorkerThread:
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
    }

    Task* Dispatcher::popTask()
    {
        Task* task = nullptr;
        u32 threadID = Dispatcher::s_threadID;

        //Check priority list first
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


        //Own thread list
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

    void Dispatcher::run(Task* task)
    {
        task->m_call(task->m_memBlock);
        task->m_destruct(task->m_memBlock);
    }

    void Dispatcher::freeTask(Task* task)
    {
        task->m_call = nullptr;
        task->m_destruct = nullptr;
#if DEBUG
        memset(task->m_memBlock, 0, k_memoryBlockSize);
#endif // DEBUG
        task->m_name = "";

        m_freeTaskMutex.lock();
        m_freeTaskList.push_back(task);
        m_freeTaskMutex.unlock();
    }

    bool Dispatcher::wait()
    {
        //for (s32 index = 0; index < m_taskQueue.size(); ++index)
        //{
        //    if (!m_taskQueue[index]->_mutex.try_lock())
        //    {
        //        //unlock all
        //        --index;
        //        for (; index >= 0; --index)
        //        {
        //            m_taskQueue[index]->_mutex.unlock();
        //        }

        //        return false;
        //    }

        //    if (!m_taskQueue[index]->_tasks.empty())
        //    {
        //        for (; index >= 0; --index)
        //        {
        //            m_taskQueue[index]->_mutex.unlock();
        //        }

        //        return false;
        //    }
        //}

        return true;
    }

    void Dispatcher::lockQueues()
    {
        for (auto iter = m_taskQueue.begin(); iter != m_taskQueue.end(); ++iter)
        {
            //(*iter)->_mutex.lock();
        }
    }

    void Dispatcher::unlockQueues()
    {
        for (auto iter = m_taskQueue.rbegin(); iter != m_taskQueue.rend(); ++iter)
        {
            //(*iter)->_mutex.unlock();
        }
    }

} //namespace task
} //namespace v3d