#pragma once

#include "Common.h"
#if TRACY_ENABLE
#   include "../ThirdParty/tracy/public/tracy/Tracy.hpp"
#endif

namespace v3d
{
} //namespace v3d

#if FRAME_PROFILER_ENABLE && defined(TRACY_ENABLE)
#   define TRACE_PROFILER_FRAME FrameMark
#   define TRACE_PROFILER_FRAME_BEGIN FrameMarkStart("Frame")
#   define TRACE_PROFILER_FRAME_END FrameMarkEnd("Frame")
#   define TRACE_PROFILER_SCOPE(name, color) ZoneScopedNC(name, color.getBGRA())

#   if 0
#       define TRACE_PROFILER_RENDER_SCOPE TRACE_PROFILER_SCOPE
#   else
#       define TRACE_PROFILER_RENDER_SCOPE
#   endif
#else
#   define TRACE_PROFILER_FRAME
#   define TRACE_PROFILER_FRAME_BEGIN
#   define TRACE_PROFILER_FRAME_END
#   define TRACE_PROFILER_SCOPE(name, color)
#endif
