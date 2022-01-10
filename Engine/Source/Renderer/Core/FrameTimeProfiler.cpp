#include "FrameTimeProfiler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

FrameTimeProfiler::FrameTimeProfiler() noexcept
    : m_counterFPS(0)
    , m_frameTime(0.0)
{
}

void FrameTimeProfiler::start()
{
    m_active = true;
}

void FrameTimeProfiler::stop()
{
    m_active = false;
}

void FrameTimeProfiler::update(f32 dt)
{
    m_frameTime += dt;
    ++m_counterFPS;
}

void FrameTimeProfiler::reset()
{
    m_frameTime = 0;
    m_counterFPS = 0;
}

u32 FrameTimeProfiler::getAverageFPS() const
{
    return m_counterFPS;
}

f32 FrameTimeProfiler::getFrameTime() const
{
    return m_frameTime / m_counterFPS;
}

void FrameTimeProfiler::printToLog() const
{
    LOG_INFO("FrameTimeProfiler: FPS: %u (%.1f ms)", FrameTimeProfiler::getAverageFPS(), FrameTimeProfiler::getFrameTime() * 1000.0f);
}

} //namespace renderer
} //namespace v3d