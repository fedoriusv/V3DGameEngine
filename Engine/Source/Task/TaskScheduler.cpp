#include "TaskScheduler.h"

namespace v3d
{
namespace task
{

TaskScheduler::TaskScheduler(u32 numWorkerThreads, u32 mask) noexcept
    : m_dispatcher(numWorkerThreads, mask)
{
}

void TaskScheduler::mainThreadLoop()
{
    ASSERT(utils::Thread::getCurrentThread() == utils::Thread::getMainThreadId(), "must be main thread");
    while (true)
    {
        Task* task = m_dispatcher.popTask();
        if (!task)
        {
            break;
        }

        m_dispatcher.run(task);
    }
}

void TaskScheduler::executeTask(Task* task, TaskPriority priority, TaskMask mask)
{
    m_dispatcher.pushTask(task, priority, mask);
}

void TaskScheduler::executeTask(std::vector<Task*> tasks, TaskPriority priority, TaskMask mask)
{
    for (auto& task : tasks)
    {
        m_dispatcher.pushTask(task, priority, mask);
    }
}

void TaskScheduler::waitTask(Task* task)
{
    task->waitCompetition();
}

} //namespace task
} //namespace v3d