#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Timer class
    */
    class Timer
    {
    public:

        enum Duration
        {
            Duration_NanoSeconds,
            Duration_MicroSeconds,
            Duration_MilliSeconds,
            Duration_Seconds,
        };

        Timer() noexcept;
        virtual ~Timer();

        template<typename Duration duration = Duration_MilliSeconds>
        u64         getTime() const;

        void        start();
        void        stop();
        void        reset();

        bool        isStopped() const;

        template<typename Duration duration = Duration_MilliSeconds>
        static u64 getCurrentTime();

    private:

        template<typename Duration duration>
        u64 convertDuration(std::chrono::nanoseconds& time) const;

        mutable std::chrono::nanoseconds m_duration;
        mutable std::chrono::time_point<std::chrono::high_resolution_clock> m_beginTime;
        mutable std::chrono::time_point<std::chrono::high_resolution_clock> m_endTime;

        bool m_isStopped;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Timer::Duration duration>
    u64 Timer::getTime() const
    {
        if (!Timer::isStopped())
        {
            m_endTime = std::chrono::high_resolution_clock::now();
            m_duration += (m_endTime - m_beginTime);
        }

        return Timer::convertDuration<duration>(m_duration);
    }

    template<Timer::Duration duration>
    inline u64 Timer::getCurrentTime()
    {
        auto now = std::chrono::high_resolution_clock::now();
        return Timer::convertDuration<duration>(now.time_since_epoch());
    }

    template<typename Timer::Duration duration>
    inline u64 Timer::convertDuration(std::chrono::nanoseconds& time) const
    {
        switch (duration)
        {
            case Timer::Duration::Duration_NanoSeconds:
                return std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();

            case Timer::Duration::Duration_MicroSeconds:
                return std::chrono::duration_cast<std::chrono::microseconds>(time).count();

            case Timer::Duration::Duration_MilliSeconds:
                return std::chrono::duration_cast<std::chrono::milliseconds>(time).count();

            case Timer::Duration::Duration_Seconds:
                return std::chrono::duration_cast<std::chrono::seconds>(time).count();

            default:
                ASSERT(false, "unknown");
        };

        return std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
} //namespace utils
} //namespace v3d
