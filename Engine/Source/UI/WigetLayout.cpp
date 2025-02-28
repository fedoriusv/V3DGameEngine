#include "WigetLayout.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WigetLayout::WigetLayout(WigetHandler* handler, const std::string& title) noexcept
    : m_handler(handler)
    , m_size(1, 1)
    , m_position(0, 0)
    , m_title(title)
{
}

WigetLayout::~WigetLayout()
{
}

void WigetLayout::setSize(const math::Dimension2D& size)
{
    m_size = size;
}

void WigetLayout::setPosition(const math::Point2D& position)
{
    m_position = position;
}

void WigetLayout::setTitle(const std::string& title)
{
    m_title = title;
}

void WigetLayout::update(f32 dt)
{
    for (auto wiget = m_wigets.begin(); wiget != m_wigets.end(); ++wiget)
    {
        (*wiget)->update(m_handler, dt);
    }
}

void WigetLayout::render(renderer::CmdListRender* cmdList)
{
}

} //namespace scene
} //namespace v3d