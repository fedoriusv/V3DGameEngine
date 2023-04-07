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

        [[nodiscard]] static math::Point2D getCursorPosition();
        static void setCursorPostion(const math::Point2D& point);

        static bool setThreadName(std::thread& thread, const std::string& name);

        [[nodiscard]] static std::wstring utf8ToWide(const char* in);
        [[nodiscard]] static std::string wideToUtf8(const wchar_t* in);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
