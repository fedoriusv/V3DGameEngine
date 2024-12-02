#include "Thread.h"
#include "Platform.h"

namespace v3d
{
namespace utils
{

std::thread::id Thread::s_mainThreadId = Thread::getCurrentThread();

Thread::Thread() noexcept
    : m_isRunning(false)
    , m_name("")
    , m_callback(nullptr)
    , m_userData(nullptr)
{
}

Thread::~Thread()
{
    ASSERT(!m_isRunning.load(std::memory_order_acquire), "Thread::~Thread(): thread is running");
    m_thread.join();
}

bool Thread::run(ThreadCallback callback, void* userData)
{
    if (m_isRunning.load(std::memory_order_acquire))
    {
        return true;
    }

    m_callback = callback;
    m_userData = userData;
    m_isRunning.store(true, std::memory_order_release);
    
    std::thread oldThread = std::exchange(m_thread, std::thread([this]() -> void
        {
            m_callback(m_userData);
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

void Thread::terminate(bool wait)
{
    m_isRunning.store(false, std::memory_order_release);
    if (wait)
    {
        m_thread.join();
    }
}

bool Thread::isRunning()
{
    return m_isRunning.load(std::memory_order_acquire);
}

const std::thread::id Thread::getThreadId() const
{
    return m_thread.get_id();
}

const std::thread::id Thread::getCurrentThread()
{
    return std::this_thread::get_id();
}

const std::thread::id Thread::getMainThreadId()
{
    return s_mainThreadId;
}

void Thread::setName(const std::string& name)
{
    platform::Platform::setThreadName(m_thread, name);
    m_name = name;
}

const std::string& Thread::getName() const
{
    return m_name;
}

void Thread::setAffinityMask(u64 mask)
{
    platform::Platform::setThreadAffinityMask(m_thread, mask);
}

} //namespace utils
} //namespace v3d
