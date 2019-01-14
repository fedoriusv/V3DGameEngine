//#include "Thread.h"
//
//#ifdef _PLATFORM_WIN_
//#include <windows.h>
//const DWORD MS_VC_EXCEPTION = 0x406D1388;
//
//#pragma pack(push,8)
//typedef struct tagTHREADNAME_INFO
//{
//    DWORD dwType; // Must be 0x1000.
//    LPCSTR szName; // Pointer to name (in user addr space).
//    DWORD dwThreadID; // Thread ID (-1=caller thread).
//    DWORD dwFlags; // Reserved for future use, must be zero.
//} THREADNAME_INFO;
//#pragma pack(pop)
//
//void SetThreadName(uint32_t dwThreadID, const char* threadName)
//{
//    // DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );
//
//    THREADNAME_INFO info;
//    info.dwType = 0x1000;
//    info.szName = threadName;
//    info.dwThreadID = dwThreadID;
//    info.dwFlags = 0;
//
//    __try
//    {
//        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
//    }
//    __except (EXCEPTION_EXECUTE_HANDLER)
//    {
//    }
//}
//
//void SetThreadName(const char* threadName)
//{
//    SetThreadName(GetCurrentThreadId(), threadName);
//}
//
//void SetThreadName(std::thread* thread, const char* threadName)
//{
//    DWORD threadId = ::GetThreadId(static_cast<HANDLE>(thread->native_handle()));
//    SetThreadName(threadId, threadName);
//}
//
//#elif _PLATFORM_LINUX_
//void SetThreadName(std::thread* thread, const char* threadName)
//{
//    auto handle = thread->native_handle();
//    pthread_setname_np(handle, threadName);
//}
//
//
//#include <sys/prctl.h>
//void SetThreadName(const char* threadName)
//{
//    prctl(PR_SET_NAME, threadName, 0, 0, 0);
//}
//
//#endif //_PLATFORM_LINUX_
//
//namespace v3d
//{
//namespace utils
//{
//std::thread::id Thread::s_mainThreadId = Thread::getCurrentThread();
//
//Thread::Thread()
//    : m_isRunning(false)
//    , m_name("")
//    , m_callback(nullptr)
//    , m_userData(nullptr)
//{
//}
//
//Thread::~Thread()
//{
//    ASSERT(!m_isRunning, "Thread::~Thread(): thread is running");
//}
//
//bool Thread::run(ThreadCallback callback, void* userData)
//{
//    if (m_isRunning)
//    {
//        return true;
//    }
//
//    m_callback = callback;
//    m_userData = userData;
//    m_isRunning = true;
//
//    m_thread = std::thread(callback, userData);
//    if (!m_thread.joinable())
//    {
//        Thread::terminate();
//    }
//
//    return m_isRunning;
//}
//
//void Thread::wait(bool terminate)
//{
//    if (m_isRunning)
//    {
//        if (terminate)
//        {
//            Thread::terminate();
//        }
//        m_thread.join();
//        m_isRunning = false;
//    }
//}
//
//void Thread::terminate()
//{
//    m_isRunning = false;
//}
//
//bool Thread::isRunning()
//{
//    return m_isRunning;
//}
//
//const std::thread::id Thread::getThreadId() const
//{
//    return m_thread.get_id();
//}
//
//const std::thread::id Thread::getCurrentThread()
//{
//    return std::this_thread::get_id();
//}
//
//const std::thread::id Thread::getMainThreadId()
//{
//    return s_mainThreadId;
//}
//
//void Thread::setName(const std::string& name)
//{
//    m_name = name;
//    SetThreadName(name.c_str());
//}
//
//const std::string & Thread::getName() const
//{
//    return m_name;
//}
//
//} //namespace utils
//} //namespace v3d
