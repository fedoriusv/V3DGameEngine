#pragma once

#include "Common.h"

#if defined(TRACY_ENABLE)
#if !TRACY_ON_DEMAND
#   error "Tracy is used record of all session. Huge memory allocation is possible. Uncomment it if you aware"
#endif
#   include "../ThirdParty/tracy/public/tracy/Tracy.hpp"
#endif //TRACY_ENABLE


namespace v3d
{
} //namespace v3d

/////////////////////////////////////////////////////////////////////////////////////////////////////

#if FRAME_PROFILER_ENABLE && defined(TRACY_ENABLE)

#define TRACE_PROFILER_FRAME FrameMark
#define TRACE_PROFILER_FRAME_BEGIN FrameMarkStart("Frame")
#define TRACE_PROFILER_FRAME_END FrameMarkEnd("Frame")
#define TRACE_PROFILER_SCOPE(name, color) ZoneScopedNC(name, color.getBGRA())

#if 0 //Memory profile
#    define TRACE_PROFILER_MEMORY_ALLOC(ptr, size, name) TracyAlloc(ptr, size)
#    define TRACE_PROFILER_MEMORY_FREE(ptr, name) TracyFree(ptr)
#else
#    define TRACE_PROFILER_MEMORY_ALLOC(ptr, size, name)
#    define TRACE_PROFILER_MEMORY_FREE(ptr, name)
#endif

#if 0 //Profile render core
#    define TRACE_PROFILER_RENDER_SCOPE TRACE_PROFILER_SCOPE
#else
#    define TRACE_PROFILER_RENDER_SCOPE
#endif

#if 0 //Trace vulkan commands
#   define TRACE_PROFILER_VULKAN_SCOPE TRACE_PROFILER_SCOPE
#else
#   define TRACE_PROFILER_VULKAN_SCOPE
#endif

#define TRACE_PROFILER_GPU_ENABLE 0
#if TRACE_PROFILER_GPU_ENABLE //Trace GPU
#   define TRACE_PROFILER_GPU_SCOPE(render, cmd, name, color) Tracy##render##ZoneC(cmd->m_tracyContext, cmd->getHandle(), name, color)
#else
#   define TRACE_PROFILER_GPU_SCOPE(render, cmd, name, color)
#endif //TRACE_PROFILER_GPU_ENABLE

#else //TRACY_ENABLE

#define TRACE_PROFILER_FRAME
#define TRACE_PROFILER_FRAME_BEGIN
#define TRACE_PROFILER_FRAME_END
#define TRACE_PROFILER_SCOPE(name, color)

#define TRACE_PROFILER_MEMORY_ALLOC(ptr, size, name)
#define TRACE_PROFILER_MEMORY_FREE(ptr, name)

#define TRACE_PROFILER_RENDER_SCOPE
#define TRACE_PROFILER_VULKAN_SCOPE

#define TRACE_PROFILER_GPU_SCOPE(context, cmd, name, color)

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
