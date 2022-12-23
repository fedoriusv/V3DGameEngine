#include "GPUTimeProfiler.h"

namespace v3d
{
namespace renderer
{

//GPUTimeProfiler::GPUTimeProfiler() noexcept
//    : m_frameStart(new Query(std::bind(&GPUTimeProfiler::responseFrametime, this, std::placeholders::_1, std::placeholders::_2)))
//    , m_frameEnd(new Query(std::bind(&GPUTimeProfiler::responseFrametime, this, std::placeholders::_1, std::placeholders::_2)))
//{
//}
//
//GPUTimeProfiler::~GPUTimeProfiler()
//{
//    if (m_frameStart)
//    {
//        delete m_frameStart;
//        m_frameStart = nullptr;
//    }
//
//    if (m_frameEnd)
//    {
//        delete m_frameEnd;
//        m_frameEnd = nullptr;
//    }
//}
//
//void GPUTimeProfiler::start()
//{
//}
//
//void GPUTimeProfiler::stop()
//{
//}
//
//void GPUTimeProfiler::update(f32 dt)
//{
//}
//
//void GPUTimeProfiler::reset()
//{
//}
//
//void GPUTimeProfiler::printToLog() const
//{
//}
//
//void GPUTimeProfiler::attachBeginFrame(RenderQuery& query)
//{
//    m_frameStart->attach(query);
//}
//
//void GPUTimeProfiler::attachEndFrame(RenderQuery& query)
//{
//    m_frameEnd->attach(query);
//}
//
//void GPUTimeProfiler::responseFrametime(const RenderQuery* query, const void* data)
//{
//
//}

} //namespace renderer
} //namespace v3d