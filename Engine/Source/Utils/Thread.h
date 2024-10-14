#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef std::function<void(void*)> ThreadCallback;

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

        bool                    run(ThreadCallback callback, void* userData);
        void                    terminate(bool wait = false);

        bool                    isRunning();

        const std::thread::id   getThreadId() const;

        void                    setName(const std::string& name);
        const std::string&      getName() const;

    private:

        std::thread             m_thread;
        std::atomic_bool        m_isRunning;

        std::string             m_name;

        void*                   m_userData;
        ThreadCallback          m_callback;

        static std::thread::id  s_mainThreadId;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
