#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

class Semaphore
{
public:
    explicit Semaphore(bool signaled = false) noexcept;

    void notify();
    void wait();

    void reset();

private:

    std::mutex              m_mutex;
    std::condition_variable m_cv;
    bool                    m_signaled;
};

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
