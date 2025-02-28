#include "WigetHandler.h"
#include "Utils/Logger.h"
#if USE_IMGUI
#include "ImGui.h"
#endif //USE_IMGUI

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

void WigetHandler::update(platform::Window* window, f32 dt)
{
    for (auto& layout : m_wigetLayouts)
    {
        layout->update(dt);
    }
}

void WigetHandler::render(renderer::CmdListRender* cmdList)
{
    for (auto& layout : m_wigetLayouts)
    {
        layout->render(cmdList);
    }
}

} // namespace ui
} // namespace v3d