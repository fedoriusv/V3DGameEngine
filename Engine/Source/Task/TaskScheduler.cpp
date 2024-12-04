#include "TaskScheduler.h"

namespace v3d
{
namespace task
{

TaskScheduler::TaskScheduler(u32 numWorkerThreads) noexcept
    : m_dispatcher(numWorkerThreads, 0)
{
}

void TaskScheduler::mainThreadLoop()
{
    ASSERT(utils::Thread::getCurrentThread() == utils::Thread::getMainThreadId(), "must be main thread");
    Task* task = m_dispatcher.popTask();
    if (task)
    {
        m_dispatcher.run(task);
        m_dispatcher.freeTask(task);
    }
}

} //namespace task
} //namespace v3d