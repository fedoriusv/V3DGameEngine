#pragma once

#include "Common.h"

namespace v3d
{
class V3D_API Application
{
public:

    Application(s32& argc, c8** argv) noexcept;
    virtual ~Application();

    virtual s32 execute();
};
} //namespace v3d