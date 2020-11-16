#pragma once

#include "AndroidCommon.h"

struct android_app;

namespace v3d
{
namespace android
{
    struct AndroidSystemProperties
    {
        std::string _productModel;
        std::string _productManufacturer;
        std::string _productDevice;
        std::string _buildVersionRelease;
    };

    inline AndroidSystemProperties g_androidSystemProperties = {};

} //namespace android
} //namespace v3d

extern "C" void android_main(android_app* state);