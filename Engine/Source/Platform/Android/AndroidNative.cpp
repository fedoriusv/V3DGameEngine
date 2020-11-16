#include "AndroidNative.h"
#include "Utils/Logger.h"

#include <android/log.h>
#include <sys/system_properties.h>
#include "android_native_app_glue.h"

extern int main(int argc, char* argv[]);

android_app* g_nativeAndroidApp;
void android_main(android_app* state)
{
    LOG_INFO("Android main started");
    g_nativeAndroidApp = state;

    //https://stackoverflow.com/questions/7183627/retrieve-android-os-build-system-properties-via-purely-native-android-app
    char tempStr[32];
    int size = 0;
    size = __system_property_get("ro.product.model", tempStr);
    v3d::android::g_androidSystemProperties._productModel.assign(tempStr, size);
    size = __system_property_get("ro.product.manufacturer", tempStr);
    v3d::android::g_androidSystemProperties._productManufacturer.assign(tempStr, size);
    size = __system_property_get("ro.product.device", tempStr);
    v3d::android::g_androidSystemProperties._productDevice.assign(tempStr, size);
    size = __system_property_get("ro.build.version.release", tempStr);
    v3d::android::g_androidSystemProperties._buildVersionRelease.assign(tempStr, size);

    LOG_DEBUG("Android product.model: %s", v3d::android::g_androidSystemProperties._productModel.data());
    LOG_DEBUG("Android product.manufacturer: %s", v3d::android::g_androidSystemProperties._productManufacturer.data());
    LOG_DEBUG("Android product.device: %s", v3d::android::g_androidSystemProperties._productDevice.data());
    LOG_DEBUG("Android build.version.release: %s", v3d::android::g_androidSystemProperties._buildVersionRelease.data());

    int argc = 0;
    char** argv = nullptr;
    main(argc, argv);
}

void android_log_arg(v3d::utils::Logger::LoggerType type, const char* message, va_list ap)
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

    __android_log_vprint(logType[type], "V3D", message, ap);
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
    __android_log_print(logType[type], "V3D", message, args);
}