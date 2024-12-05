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

        class Batcher
        {
        public:

            Batcher(TaskPriority priority, TaskMask mask) noexcept
                : _priority(priority)
                , _mask(mask)
            {
            }

            template<typename Func, typename ...Args>
            void addTask(Func&& func, Args && ...args)
            {
                //auto fn = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
                //_funcs.push_back(fn);
            }

            TaskPriority        _priority;
            TaskMask            _mask;
            std::vector<Task*>  _funcs;
        };

        explicit TaskScheduler(u32 numWorkerThreads) noexcept;

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

