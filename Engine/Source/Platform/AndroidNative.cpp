#include "AndroidNative.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_ANDROID
#include <android/log.h>

extern int main(int argc, char* argv[]);

struct android_app* g_nativeAndroidApp;
void android_main(struct android_app* state)
{
    LOG_INFO("Android main started");

    //app_dummy();
    g_nativeAndroidApp = state;

    //TODO
    main(0, nullptr);
}

void android_log(v3d::utils::Logger::LoggerType type, const char* message, ...)
{
    static android_LogPriority logType[v3d::utils::Logger::LoggerType::LoggerCount] = 
    {
        ANDROID_LOG_DEBUG,
        ANDROID_LOG_VERBOSE,
        ANDROID_LOG_INFO,
        ANDROID_LOG_WARN,
        ANDROID_LOG_ERROR,
        ANDROID_LOG_FATAL
    };

    va_list args;
    va_start(args, message);
    __android_log_vprint(logType[type], "V3D", message, args);
    va_end(args);
}

#endif //PLATFORM_ANDROID