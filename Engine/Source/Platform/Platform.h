#pragma once

#include "Common.h"

namespace v3d
{
namespace platform
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Window;

    using DisplayMonitorsFunc = std::function<bool(const math::RectF32& rcMonitor, const math::RectF32& rcWork, f32 dpi, bool primary, void* monitor)>;

    /**
    * @brief Platform class. Platform specific functions 
    */
    class V3D_API Platform final
    {
    public:

        [[nodiscard]] static math::Point2D getCursorPosition();
        static void setCursorPostion(const math::Point2D& point);

        static bool setThreadName(std::thread& thread, const std::string& name);
        static bool setThreadAffinityMask(std::thread& thread, u64 mask);

        [[nodiscard]] static std::wstring utf8ToWide(const c8* in);
        [[nodiscard]] static std::string wideToUtf8(const w16* in);

        static void enumDisplayMonitors(const DisplayMonitorsFunc& func);
        static f32 getDpiScaleForWindow(const Window* window);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
