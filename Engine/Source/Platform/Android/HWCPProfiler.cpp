#include "HWCPProfiler.h"
#include "Utils/Logger.h"

#ifdef USE_HWC_PIPE
namespace v3d
{
namespace android
{

const std::string HWCPProfiler::s_cpuCounterNames[] =
{
    "Cycles",
    "Instructions",
    "CacheReferences",
    "CacheMisses",
    "BranchInstructions",
    "BranchMisses",
    "L1Accesses",
    "InstrRetired",
    "L2Accesses",
    "L3Accesses",
    "BusReads",
    "BusWrites",
    "MemReads",
    "MemWrites",
    "ASESpec",
    "VFPSpec",
    "CryptoSpec"
};

const std::string HWCPProfiler::s_gpuCounterNames[] =
{
    "GpuCycles",
    "VertexComputeCycles",
    "FragmentCycles",
    "TilerCycles",
    "VertexComputeJobs",
    "FragmentJobs",
    "Pixels",
    "Tiles",
    "TransactionEliminations",
    "EarlyZTests",
    "EarlyZKilled",
    "LateZTests",
    "LateZKilled",
    "Instructions",
    "DivergedInstructions",
    "ShaderCycles",
    "ShaderArithmeticCycles",
    "ShaderLoadStoreCycles",
    "ShaderTextureCycles",
    "CacheReadLookups",
    "CacheWriteLookups",
    "ExternalMemoryReadAccesses",
    "ExternalMemoryWriteAccesses",
    "ExternalMemoryReadStalls",
    "ExternalMemoryWriteStalls",
    "ExternalMemoryReadBytes",
    "ExternalMemoryWriteBytes"
};

HWCPProfiler::HWCPProfiler(const std::vector<CpuCounter>& cpuCounters, const std::vector<GpuCounter>& gpuCounters) noexcept
    : m_HWCPipe()
{
    static_assert((u32)CpuCounter::MaxValue == (u32)hwcpipe::CpuCounter::MaxValue, "differrent size");
    static_assert((u32)GpuCounter::MaxValue == (u32)hwcpipe::GpuCounter::MaxValue, "differrent size");

    //CPU
    if (m_HWCPipe.cpu_profiler())
    {
        const hwcpipe::CpuCounterSet& supportedCpuCounters = m_HWCPipe.cpu_profiler()->supported_counters();
        LOG_DEBUG("HWCPProfiler: Supported cpu counters %u", static_cast<u32>(supportedCpuCounters.size()));
        hwcpipe::CpuCounterSet enabledCpuSet;
        for (auto& counter : cpuCounters)
        {
            auto foundCounter = supportedCpuCounters.find(hwcpipe::CpuCounter(counter));
            if (foundCounter != supportedCpuCounters.cend())
            {
                enabledCpuSet.insert(*foundCounter);
                LOG_DEBUG("HWCPProfiler: Cpu counter is supported %s", HWCPProfiler::s_cpuCounterNames[toEnumType(counter)].c_str());
            }
            else
            {
                LOG_WARNING("HWCPProfiler: Cpu counter is not supported %s", HWCPProfiler::s_cpuCounterNames[toEnumType(counter)].c_str());
            }
        }
        if (!enabledCpuSet.empty())
        {
            m_HWCPipe.set_enabled_cpu_counters(enabledCpuSet);
        }
    }
    else
    {
        LOG_DEBUG("HWCPProfiler: cpu_profiler is not supported");
    }

    //GPU
    if (m_HWCPipe.gpu_profiler())
    {
        const hwcpipe::GpuCounterSet& supportedGpuCounters = m_HWCPipe.gpu_profiler()->supported_counters();
        LOG_DEBUG("HWCPProfiler: Supported gpu counters %u", static_cast<u32>(supportedGpuCounters.size()));
        hwcpipe::GpuCounterSet enabledGpuSet;
        for (auto& counter : gpuCounters)
        {
            auto foundCounter = supportedGpuCounters.find(hwcpipe::GpuCounter(counter));
            if (foundCounter != supportedGpuCounters.cend())
            {
                enabledGpuSet.insert(*foundCounter);
                LOG_DEBUG("HWCPProfiler: Gpu counter is supported %s", HWCPProfiler::s_gpuCounterNames[toEnumType(counter)].c_str());
            }
            else
            {
                auto unsupportedCounter = hwcpipe::gpu_counter_info.find(*foundCounter);
                ASSERT(unsupportedCounter != hwcpipe::gpu_counter_info.cend(), "not found");
                LOG_WARNING("HWCPProfiler: Gpu counter is not supported %s", HWCPProfiler::s_gpuCounterNames[toEnumType(counter)].c_str());
            }
        }
        if (!enabledGpuSet.empty())
        {
            m_HWCPipe.set_enabled_gpu_counters(enabledGpuSet);
        }
    }
    else
    {
        LOG_DEBUG("HWCPProfiler: gpu_profiler is not supported");
    }

    m_HWCPipe.run();
}

HWCPProfiler::~HWCPProfiler()
{
    ASSERT(isActive(), "still works");
    m_HWCPipe.stop();
}

void HWCPProfiler::start()
{
    m_active = true;
}

void HWCPProfiler::stop()
{
    m_active = false;
}

void HWCPProfiler::update(f32 dt)
{
    //LOG_DEBUG("HWCPProfiler: call every frame %f", dt);
    const hwcpipe::Measurements& measurements = m_HWCPipe.sample();

    if (measurements.cpu)
    {
        //LOG_DEBUG("HWCPProfiler: Update profiler measurements.cpu %d", measurements.cpu->size());
        for (auto& counter : *measurements.cpu)
        {
            auto iter = _statistic.emplace(toEnumType(counter.first), Counter());
            Counter& statistic = iter.first->second;
            if (iter.second)
            {
                statistic._type = 0;
                statistic._index = toEnumType(counter.first);
                statistic._name =  HWCPProfiler::s_cpuCounterNames[toEnumType(counter.first)];
                auto cpuCounter = hwcpipe::cpu_counter_info.find(counter.first);
                ASSERT(cpuCounter != hwcpipe::cpu_counter_info.cend(), "not found");
                statistic._desctiption = cpuCounter->second.desc;
                statistic._unit = cpuCounter->second.unit;
                statistic._value = counter.second.get<f64>();
            }

            statistic._value = counter.second.get<f64>();
            if (m_factor == IntervalFactor::PerSecond && dt != 0.0f)
            {
                statistic._value /= dt;
            }
        }
    }

    if (measurements.gpu)
    {
        //LOG_DEBUG("HWCPProfiler: Update profiler measurements.gpu %d", measurements.gpu->size());
        for (auto& counter : *measurements.gpu)
        {
            auto iter = _statistic.emplace(toEnumType(counter.first) + toEnumType(CpuCounter::MaxValue), Counter());
            Counter &statistic = iter.first->second;
            if (iter.second)
            {
                statistic._type = 0;
                statistic._index = toEnumType(counter.first);
                statistic._name =  HWCPProfiler::s_gpuCounterNames[toEnumType(counter.first)];
                auto gpuCounter = hwcpipe::gpu_counter_info.find(counter.first);
                ASSERT(gpuCounter != hwcpipe::gpu_counter_info.cend(), "not found");
                statistic._desctiption = gpuCounter->second.desc;
                statistic._unit = gpuCounter->second.unit;
                statistic._value = counter.second.get<f64>();
            }

            statistic._value = counter.second.get<f64>();
            if (m_factor == IntervalFactor::PerSecond && dt != 0.0f)
            {
                statistic._value /= dt;
            }
        }
    }
}

void HWCPProfiler::reset()
{
    //LOG_DEBUG("HWCPProfiler: call every 1 sec");
    printToLog();
}

void HWCPProfiler::printToLog() const
{
    LOG_DEBUG("HWCPProfiler: Statistic %d", _statistic.size());
    for (auto& counter : _statistic)
    {
        const Counter& statistic = counter.second;
        if (statistic._unit == "B")
        {
            if (m_factor == IntervalFactor::PerSecond)
            {
                LOG_DEBUG("HWCPProfiler [%s] %s: %f %s/Sec", statistic._name.c_str(), statistic._desctiption.c_str(), statistic._value, statistic._unit.c_str());
            }
            else
            {
                LOG_DEBUG("HWCPProfiler [%s] %s: %f %s", statistic._name.c_str(), statistic._desctiption.c_str(), statistic._value, statistic._unit.c_str());
            }
        }
        else
        {
            u64 value = static_cast<u64>(std::ceil(statistic._value));
            if (m_factor == IntervalFactor::PerSecond)
            {
                LOG_DEBUG("HWCPProfiler [%s] %s: %u %s/Sec", statistic._name.c_str(), statistic._desctiption.c_str(), value, statistic._unit.c_str());
            }
            else
            {
                LOG_DEBUG("HWCPProfiler [%s] %s: %u %s", statistic._name.c_str(), statistic._desctiption.c_str(), value, statistic._unit.c_str());
            }
        }
    }

    /*
    //cycles == hz;
    LOG_DEBUG("HWCPProfiler: General");
    {
        auto JM_GPU_ACTIVE = statistic.find(toEnumType(GpuCounter::GpuCycles) + toEnumType(CpuCounter::MaxValue));
        auto JM_JS1_ACTIVE = statistic.find(toEnumType(GpuCounter::VertexComputeCycles) + toEnumType(CpuCounter::MaxValue));
        auto JM_JS0_ACTIVE = statistic.find(toEnumType(GpuCounter::FragmentCycles) + toEnumType(CpuCounter::MaxValue));
        JM_GPU_UTILIZATION = JM_GPU_ACTIVE / GPU_MHZ;
        JM_JS0_UTILIZATION = JM_JS0_ACTIVE / JM_GPU_ACTIVE
    }
    LOG_DEBUG("HWCPProfiler: GPU Utilization - Vertex: %d%, Fragment: %d%", )
    */
}

} //namespace android
} //namespace v3d
#endif