#include "Semaphore.h"
#include "utils/Logger.h"

namespace v3d
{
namespace utils
{

Semaphore::Semaphore(bool signaled) noexcept
    : m_signaled(signaled)
{
}

void Semaphore::notify()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_signaled = true;
    m_cv.notify_one();
}

void Semaphore::wait()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (!m_signaled)
    {
        m_cv.wait(lock);
    }
}

void Semaphore::reset()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_signaled = false;
}

} //namespace utils
} //namespace v3d
