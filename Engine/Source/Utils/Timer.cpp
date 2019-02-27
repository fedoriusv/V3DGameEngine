//#include "Timer.h"
//#ifdef _PLATFORM_WIN_
//#   include <windows.h>
//#else //_PLATFORM_WIN_
//#   include <sys/time.h>  
//#endif //_PLATFORM_WIN_
//
//namespace v3d
//{
//namespace utils
//{
//
//s64 CTimer::getTicks()
//{
//#ifdef _PLATFORM_WIN_
//    LARGE_INTEGER ticks;
//    QueryPerformanceCounter(&ticks);
//
//    return ticks.QuadPart;
//#else //_PLATFORM_WIN_
//    struct timeval tp;
//    gettimeofday(&tp, 0);
//    s64 time = s64(tp.tv_sec) * 1000000 + tp.tv_usec;
//
//    return time;
//#endif //_PLATFORM_WIN_
//}
//
//s64 CTimer::getTicksPerSecond()
//{
//#ifdef _PLATFORM_WIN_
//    LARGE_INTEGER tickPerSeconds;
//    QueryPerformanceFrequency(&tickPerSeconds);
//
//    return tickPerSeconds.QuadPart;
//#else //_PLATFORM_WIN_
//    return 1000000;
//#endif //_PLATFORM_WIN_
//}
//
//s64 CTimer::getCurrentTime() //in Milliseconds
//{
//    return s64(((f64)CTimer::getTicks() / (f64)CTimer::getTicksPerSecond()) * 1000.0);
//}
//
//CTimer::CTimer()
//: m_lastTime(0)
//, m_currentTime(0)
//, m_time(0)
//, m_isStopped(false)
//{
//    m_time = CTimer::getCurrentTime();
//}
//
//CTimer::~CTimer()
//{
//}
//
//u32 CTimer::getTime() const
//{
//    if (CTimer::isStopped())
//    {
//        return m_lastTime;
//    }
//
//    return m_lastTime + (u32)(m_time - m_currentTime);
//}
//
//void CTimer::setTime(u32 time)
//{
//    m_time = CTimer::getCurrentTime();
//    m_lastTime = time;
//    m_currentTime = m_time;
//}
//
//void CTimer::start()
//{
//    if (!CTimer::isStopped())
//    {
//        CTimer::setTime(m_lastTime);
//    }
//
//    m_isStopped = false;
//}
//
//void CTimer::stop()
//{
//    if (!CTimer::isStopped())
//    {
//        m_lastTime = CTimer::getTime();
//    }
//
//    m_isStopped = true;
//}
//
//bool CTimer::isStopped() const
//{
//    return m_isStopped;
//}
//
//} //namespace utils
//} //namespace v3d
