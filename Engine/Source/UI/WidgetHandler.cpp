#include "WidgetHandler.h"
#include "WidgetWindow.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WidgetHandler::WidgetHandler(renderer::Device* device) noexcept
    : m_device(device)
    , m_uiDrawer(nullptr)
{
}

WidgetHandler::~WidgetHandler()
{
}

void WidgetHandler::update(const platform::Window* window, const v3d::event::InputEventHandler* handler, f32 dt)
{
    for (Widget* widget : m_widgets)
    {
        if (widget->isVisible())
        {
            widget->update(this, nullptr, nullptr, dt);
        }
    }
}

bool WidgetHandler::render(renderer::CmdListRender* cmdList)
{
    return false;
}

} // namespace ui
} // namespace v3d