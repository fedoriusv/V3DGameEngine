#include "Task.h"

namespace v3d
{
namespace task
{

TaskCounter::TaskCounter(u32 value) noexcept
    : m_counter(value)
{
}

TaskCounter::~TaskCounter()
{
    ASSERT(m_counter.load(std::memory_order_relaxed) == 0, "must be free");
}

u32 TaskCounter::wait()
{
    return u32();
}

void TaskCounter::increment()
{
    m_counter.fetch_add(1);
}

void TaskCounter::decrement()
{
}

bool TaskCounter::compare_exchange(u32 prev, u32 next)
{
    return m_counter.compare_exchange_strong(prev, next);
}

} // namespace task
} // namespace v3d