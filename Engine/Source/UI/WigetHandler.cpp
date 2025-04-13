#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WigetHandler::WigetHandler(renderer::Device* device) noexcept
    : m_device(device)
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

void WigetHandler::render(renderer::CmdListRender* cmdList)
{
    //do nothing
}

} // namespace ui
} // namespace v3d