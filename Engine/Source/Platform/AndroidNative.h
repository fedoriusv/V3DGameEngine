#pragma once

#include "Configuration.h"

#ifdef PLATFORM_ANDROID

#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android/sensor.h>

extern "C" void android_main(struct android_app* state);

#endif //PLATFORM_ANDROID