#pragma once

#include "Common.h"
#include "Singleton.h"
#include "Timer.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Profiler class.
    */
    class Profiler
    {
    public:

        enum class IntervalFactor
        {
            PerFrame,
            PerSecond,
            Counter
        };

        Profiler() noexcept = default;
        virtual ~Profiler() = default;

        bool isActive() const;

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual void update(f32 dt) = 0;
        virtual void reset() = 0;

    protected:

        const IntervalFactor m_factor = IntervalFactor::PerFrame;
        bool m_active = false;
    };

    /**
    * @brief ProfilerManager class. Handles Profiler
    */
    class ProfileManager final : public Singleton<ProfileManager>
    {
    public:

        void attach(Profiler* profiler);
        bool dettach(Profiler* profiler);

        void start();
        void stop();

        void update();

    private:

        /**
        * @brief reset counter every n miliseconds.
        */
        const u32 k_resetTime = 1000 * 1;

        ProfileManager() noexcept = default;
        ~ProfileManager();

        std::set<Profiler*> m_profilers;
        utils::Timer m_timer;

        friend Singleton<ProfileManager>;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d