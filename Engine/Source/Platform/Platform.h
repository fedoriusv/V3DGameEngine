#pragma once

#include "Common.h"

namespace v3d
{
namespace platform
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Window;

    using DisplayMonitorsFunc = std::function<bool(const math::Rect& rcMonitor, const math::Rect& rcWork, f32 dpi, bool primary, void* monitor)>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Platform class. Platform specific functions 
    */
    class V3D_API Platform final
    {
    public:

        enum class CursorIcon
        {
            Arrow,
            TextInput,
            ResizeAll,
            ResizeEW,
            ResizeNS,
            ResizeNESW,
            ResizeNWSE,
            Hand,
            Wait,
            Progress,
            NotAllowed,
        };

        static void setCursorIcon(CursorIcon icon);
        static void showCursor();
        static void hideCursor();

        [[nodiscard]] static math::Point2D getCursorPosition();
        static void setCursorPostion(const math::Point2D& point);

        static bool setThreadName(std::thread& thread, const std::string& name);
        static bool setThreadAffinityMask(std::thread& thread, u64 mask);
        static bool setThreadPriority(std::thread& thread, s32 priority);

        [[nodiscard]] static std::wstring utf8ToWide(const c8* in);
        [[nodiscard]] static std::string wideToUtf8(const w16* in);

        static void enumDisplayMonitors(const DisplayMonitorsFunc& func);
        static f32 getDpiScaleForWindow(const Window* window);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
