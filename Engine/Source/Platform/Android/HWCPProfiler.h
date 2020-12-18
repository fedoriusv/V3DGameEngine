#pragma once

#include "Common.h"
#include "Utils/Profiler.h"

#ifdef USE_HWC_PIPE
#   include "HWCPipe/hwcpipe.h"

namespace v3d
{
namespace android
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief HWCPProfiler class.
    * Uses only for ARM devices.
    * @see hwcpipe lib
    * If counters isn't enabled use the next command:
    * adb shell setprop security.perf_harden 0
    * @see utils::Profiler
    */
    class HWCPProfiler : public utils::Profiler
    {
    public:

        enum class CpuCounter : u32
        {
            Cycles,
            Instructions,
            CacheReferences,
            CacheMisses,
            BranchInstructions,
            BranchMisses,

            L1Accesses,
            InstrRetired,
            L2Accesses,
            L3Accesses,
            BusReads,
            BusWrites,
            MemReads,
            MemWrites,
            ASESpec,
            VFPSpec,
            CryptoSpec,

            MaxValue
        };

        enum class GpuCounter : u32
        {
            GpuCycles,
            VertexComputeCycles,
            FragmentCycles,
            TilerCycles,

            VertexComputeJobs,
            FragmentJobs,
            Pixels,

            Tiles,
            TransactionEliminations,
            EarlyZTests,
            EarlyZKilled,
            LateZTests,
            LateZKilled,

            Instructions,
            DivergedInstructions,

            ShaderCycles,
            ShaderArithmeticCycles,
            ShaderLoadStoreCycles,
            ShaderTextureCycles,

            CacheReadLookups,
            CacheWriteLookups,
            ExternalMemoryReadAccesses,
            ExternalMemoryWriteAccesses,
            ExternalMemoryReadStalls,
            ExternalMemoryWriteStalls,
            ExternalMemoryReadBytes,
            ExternalMemoryWriteBytes,

            MaxValue
        };

        explicit HWCPProfiler(const std::vector<CpuCounter>& cpuCounters, const std::vector<GpuCounter>& gpuCounters) noexcept;
        ~HWCPProfiler();

        void start() override;
        void stop() override;

        void update(f32 dt) override;
        void reset() override;

    private:

        void printToLog() const;

        hwcpipe::HWCPipe m_HWCPipe;

        struct Counter
        {
            Counter() noexcept = default;

            u32 _type;
            u32 _index;
            
            std::string _name;
            std::string _desctiption;
            std::string _unit;
            f64 _value;
        };

        std::map<u32, Counter> _statistic;

        const static std::string s_cpuCounterNames[toEnumType(CpuCounter::MaxValue)];
        const static std::string s_gpuCounterNames[toEnumType(GpuCounter::MaxValue)];

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace android
} //namespace v3d

#endif //USE_HWC_PIPE