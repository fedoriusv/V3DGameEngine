#pragma once

#include "Common.h"
#include "Task.h"
#include "TaskDispatcher.h"

namespace v3d
{
namespace task
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TaskScheduler class
    */
    class TaskScheduler
    {
    public:

        explicit TaskScheduler(u32 numWorkerThreads) noexcept;
        ~TaskScheduler() = default;

        void mainThreadLoop();


        void executeTask(Task* task, TaskPriority priority, TaskMask mask);
        void executeTask(std::vector<Task*> tasks, TaskPriority priority, TaskMask mask);

        void waitTask(Task* task);

    private:

        TaskDispatcher m_dispatcher;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namesapce task
} // namespace v3d

