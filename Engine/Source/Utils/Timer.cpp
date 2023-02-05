#include "Timer.h"

namespace v3d
{
namespace utils
{

Timer::Timer() noexcept
    : m_isStopped(true)
{
    Timer::reset();
}

Timer::~Timer()
{
}

void Timer::start()
{
    if (Timer::isStopped())
    {
        m_beginTime = std::chrono::steady_clock::now();
    }

    m_isStopped = false;
}

void Timer::stop()
{
    if (!Timer::isStopped())
    {
        m_endTime = std::chrono::steady_clock::now();
        m_duration += (m_endTime - m_beginTime);
    }

    m_isStopped = true;
}

void Timer::reset()
{
    if (!Timer::isStopped())
    {
        m_beginTime = std::chrono::steady_clock::now();
        m_endTime = m_beginTime;
    }

    m_duration = std::chrono::duration_values<std::chrono::nanoseconds>::zero();
}

bool Timer::isStopped() const
{
    return m_isStopped;
}

} //namespace utils
} //namespace v3d
