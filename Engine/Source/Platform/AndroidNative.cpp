#include "AndroidNative.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_ANDROID

extern struct android_app* g_nativeAndroidApp;
extern int main(int argc, char* argv[]);

void android_main(struct android_app* state)
{
    LOG_INFO("Android main started");

    app_dummy();
    g_nativeAndroidApp = state;

    //TODO
    main(0, nullptr);
}

#endif //PLATFORM_ANDROID