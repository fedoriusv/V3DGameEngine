#pragma once

#include "Common.h"
#include "Utils/Profiler.h"

namespace v3d
{
namespace scene
{
    struct SceneStatistic : utils::ProfilerStaticstics
    {
        std::vector<utils::Profiler::CommonMetric> _statistic;

        void print() const override;
    };

    class SceneProfiler : public utils::Profiler
    {
    public:

        SceneProfiler() noexcept;
        ~SceneProfiler() = default;

        void start(const std::string& name);
        void stop(const std::string& name);

        void update(f32 dt) override;

    private:
        void reset() override;
        void collect() override;
        const utils::ProfilerStaticstics* statistic() const override;

        std::map<std::string, u64> m_collect;
        SceneStatistic m_stat;
    };
} //namespace scene
} //namespace v3d
