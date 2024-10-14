#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Timer class.
    * Uses for time counting
    */
    class V3D_API Timer
    {
    public:

        /**
        * @brief Duration enum
        */
        enum Duration
        {
            Duration_NanoSeconds,
            Duration_MicroSeconds,
            Duration_MilliSeconds,
            Duration_Seconds,
        };

        Timer() noexcept;
        virtual ~Timer();

        /**
        * @brief getTime method.
        * @return passed time form called start method
        */
        template<typename Timer::Duration duration = Duration_MilliSeconds>
        u64 getTime() const;

        /**
        * @brief start method.
        * Start point. If duration is not zero, time will be added
        */
        void start();

        /**
        * @brief stop method.
        * Stop point. Gets duartion form start call to stop call. Will be added to previos duration
        */
        void stop();

        /**
        * @brief reset method.
        * Reset timer. All duration resets to zero.
        */
        void reset();

        /**
        * @brief isStopped method.
        * @return status of timer
        */
        bool isStopped() const;

        /**
        * @brief getCurrentTime function.
        * @return return current time
        */
        template<typename Timer::Duration duration = Duration_MilliSeconds>
        static u64 getCurrentTime();

    private:

        template<typename Timer::Duration duration>
        static u64 convertDuration(const std::chrono::nanoseconds& time);

        mutable std::chrono::nanoseconds m_duration;
        mutable std::chrono::time_point<std::chrono::steady_clock> m_beginTime;
        mutable std::chrono::time_point<std::chrono::steady_clock> m_endTime;

        bool m_isStopped;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Timer::Duration duration>
    u64 Timer::getTime() const
    {
        if (!Timer::isStopped())
        {
            m_endTime = std::chrono::steady_clock::now();
            return Timer::convertDuration<duration>(m_duration + (m_endTime - m_beginTime));
        }

        return Timer::convertDuration<duration>(m_duration);
    }

    template<Timer::Duration duration>
    inline u64 Timer::getCurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        return Timer::convertDuration<duration>(now.time_since_epoch());
    }

    template<typename Timer::Duration duration>
    inline u64 Timer::convertDuration(const std::chrono::nanoseconds& time)
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
