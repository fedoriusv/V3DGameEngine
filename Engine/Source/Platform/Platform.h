#pragma once

#include "Common.h"

namespace v3d
{
namespace platform
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Platform final
    {
    public:

        static core::Point2D getCursorPosition();
        static void setCursorPostion(const core::Point2D& point);
        static bool setThreadName(std::thread& thread, const std::string& name);

        static std::wstring utf8ToWide(const char* in);
        static std::string wideToUtf8(const wchar_t* in);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
