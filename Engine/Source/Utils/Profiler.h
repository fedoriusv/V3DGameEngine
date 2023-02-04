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
    * @brief ProfilerStaticstics
    */
    struct ProfilerStaticstics
    {
        ProfilerStaticstics() noexcept = default;
        virtual ~ProfilerStaticstics() = default;

        virtual void print() const = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ProfileManager;

    /**
    * @brief Profiler class. Uses for render profiler
    */
    class Profiler
    {
    public:

        /**
        * @brief CommonMetric struct
        */
        struct CommonMetric
        {
            CommonMetric() noexcept = default;

            u32 _type = 0;
            u32 _index = 0;

            std::string _name;
            std::string _desctiption;
            std::string _unit;
            f64 _value = 0.0;
        };

        /**
        * @brief IntervalFactor enum. Collect counters by factor
        */
        enum class IntervalFactor
        {
            PerFrame,
            PerSecond,
            Custom
        };

        Profiler() noexcept = default;
        virtual ~Profiler() = default;

        bool isActive() const;

    protected:

        virtual void update(f32 dt) = 0;
        virtual void reset() = 0;

        virtual void collect() = 0;
        virtual const ProfilerStaticstics* statistic() const = 0;

        const IntervalFactor m_factor = IntervalFactor::PerFrame;
        bool m_active = false;

        friend ProfileManager;
    };

    /**
    * @brief ProfilerManager class. Handles Profiler
    */
    class ProfileManager final
    {
    public:

        ProfileManager() noexcept = default;
        ~ProfileManager();

        void attach(Profiler* profiler);
        bool dettach(Profiler* profiler);

        void update();
        void freeProfilers();

    private:

        std::set<Profiler*> m_profilers;
        std::vector<const ProfilerStaticstics*> m_statistic;
        utils::Timer m_timer;

        friend Singleton<ProfileManager>;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d