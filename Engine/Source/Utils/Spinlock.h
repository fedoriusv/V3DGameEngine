#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Spinlock class
    */
    class Spinlock
    {
    public:

        Spinlock() noexcept
        {
            m_flag.clear();
        }

        ~Spinlock()
        {
        }

        void lock()
        {
            while (m_flag.test_and_set(std::memory_order_acquire))
            {
                //spin lock
            }
        }

        bool try_lock()
        {
            if (!m_flag.test_and_set(std::memory_order_acquire))
            {
                return false;
            }

            return true;
        }

        void unlock()
        {
            m_flag.clear(std::memory_order_release);
        }

    private:

        std::atomic_flag m_flag;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
