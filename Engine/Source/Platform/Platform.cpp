#include "Platform.h"

namespace v3d
{
namespace platform
{

core::Point2D Platform::getCursorPosition()
{
#ifdef PLATFORM_WINDOWS
    POINT mouse;
    GetCursorPos(&mouse);

    return core::Point2D(static_cast<s32>(mouse.x), static_cast<s32>(mouse.y));
#endif //PLATFORM_WINDOWS

    return core::Point2D(0U, 0U);
}

void Platform::setCursorPostion(const core::Point2D & point)
{
#ifdef PLATFORM_WINDOWS
    SetCursorPos(point.x, point.y);
#endif //PLATFORM_WINDOWS
}

} //namespace platform
} //namespace v3d
