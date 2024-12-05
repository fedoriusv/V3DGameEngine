#pragma once

#include "Common.h"
#include "TaskContainers.h"

namespace v3d
{
namespace task
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class TaskDispatcher;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TaskPriority enum
    */
    enum class TaskPriority
    {
        Normal,
        High,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Mask enum
    */
    enum class TaskMask
    {
        AnyThread,
        MainThread,
        WorkerThread,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Task struct
    */
    class Task : protected ThreadSafeNode<Task>
    {
    public:

        enum Status
        {
            Empty,
            Created,
            Scheduled,
            Waiting,
            Executing,
            Completed,
        };

        Task() noexcept;
        ~Task() = default;

        template<typename Func, typename ...Args>
        void init(Func&& func, Args&& ...args)
        {
            m_func = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
            /*m_func = [ifn = std::move(fn)]()
                {
                    ifn();
                };*/
        }

        template<typename Func, typename ...Args>
        void init(const std::string& name, Func&& func, Args&& ...args)
        {
            m_name = name;
            m_func = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
            /*m_func = [ifn = std::move(fn)]()
                {
                    ifn();
                };*/
        }

        template<typename Func, typename ...Args>
        void cond(Func&& func, Args&& ...args)
        {
            m_cond = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
            /*m_cond = [ifn = std::move(fn)]()
                {
                    return ifn();
                };*/
        }

        bool isCompeted() const;
        const std::string& getName() const;

        void waitCompetition();

    private:

        friend TaskDispatcher;

        TaskPriority                m_priority;
        TaskMask                    m_mask;
        std::atomic<Status>         m_result;
        std::string                 m_name;
        //TODO own allocator
        std::function<void()>       m_func;
        std::function<bool()>       m_cond;
        std::condition_variable_any m_wait;
    };

    inline Task::Task() noexcept
        : m_priority(TaskPriority::Normal)
        , m_mask(TaskMask::AnyThread)
        , m_result(Empty)
    {
    }

    inline bool Task::isCompeted() const
    {
        return m_result.load(std::memory_order_relaxed) > Status::Executing;
    }

    inline const std::string& Task::getName() const
    {
        return m_name;
    }

    inline void Task::waitCompetition()
    {
        struct FakeMutex
        {
            void lock()
            {
                int b;
            };
            void unlock()
            {
                int a = 0;
            };
        };
        FakeMutex mutex;

        m_wait.wait(mutex, [this]() -> bool
            {
                return isCompeted();
            });
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namesapce task
} // namespace v3d