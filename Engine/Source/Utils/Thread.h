#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Thread class
    */
    class V3D_API Thread
    {
    public:

        Thread() noexcept;
        ~Thread();

        static const std::thread::id    getCurrentThread();
        static const std::thread::id    getMainThreadId();

        template<typename Func, typename...Args>
        bool                    run(Func&& func, Args&&... args);

        void                    terminate(bool wait = false);

        bool                    isRunning();

        const std::thread::id   getThreadId() const;

        void                    setName(const std::string& name);
        const std::string&      getName() const;

        void                    setAffinityMask(u64 mask);

    private:

        std::thread             m_thread;
        std::atomic_bool        m_isRunning;
        std::function<void()>   m_callback;

        std::string             m_name;

        static std::thread::id  s_mainThreadId;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Func, typename ...Args>
    inline bool Thread::run(Func&& func, Args && ...args)
    {
        if (m_isRunning.load(std::memory_order_acquire))
        {
            return true;
        }

        m_callback = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
        std::thread oldThread = std::exchange(m_thread, std::thread([this]() -> void
            {
                m_isRunning.store(true, std::memory_order_release);
                m_callback();
                m_isRunning.store(false, std::memory_order_release);

            }));

        if (oldThread.joinable())
        {
            oldThread.join();
        }

        if (!m_thread.joinable())
        {
            m_isRunning.store(false, std::memory_order_release);
        }

        return m_isRunning.load(std::memory_order_acquire);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
