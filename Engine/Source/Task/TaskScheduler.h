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

        template<typename Func, typename...Args>
        void executeTask(TaskPriority priority, TaskMask mask, Func&& func, Args&&... args);

        void executeBatch(Batcher& batch);

        void mainThreadLoop();

    private:

        TaskDispatcher m_dispatcher;
    };

    template<typename Func, typename ...Args>
    inline void TaskScheduler::executeTask(TaskPriority priority, TaskMask mask, Func&& func, Args && ...args)
    {
        auto fn = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
        Task* task = m_dispatcher.prepareTask(priority, mask, getFunctionType<decltype(fn)>(), &fn);
        m_dispatcher.pushTask(task);

        //return TaskResult{ task };
    }


    inline void TaskScheduler::executeBatch(TaskScheduler::Batcher& batch)
    {
        //for (auto& func : batch._funcs)
        //{
        //    Task* task = m_dispatcher.prepareTask(batch._priority, batch._mask, getFunctionType<decltype(func)>(), &func);
        //    m_dispatcher.pushTask(task);
        //}
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namesapce task
} // namespace v3d

