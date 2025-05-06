#include "WigetHandler.h"
#include "WigetWindow.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WigetHandler::WigetHandler(renderer::Device* device) noexcept
    : m_device(device)
    , m_uiDrawer(nullptr)
{
}

WigetHandler::~WigetHandler()
{
}

void WigetHandler::update(const platform::Window* window, const v3d::event::InputEventHandler* handler, f32 dt)
{
    for (Wiget* wiget : m_wigets)
    {
        if (wiget->isVisible())
        {
            wiget->update(this, nullptr, nullptr, dt);
        }
    }
}

bool WigetHandler::render(renderer::CmdListRender* cmdList)
{
    return false;
}

} // namespace ui
} // namespace v3d