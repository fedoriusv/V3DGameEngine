#include "WigetLayout.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

WigetLayout::WigetLayout() noexcept
{
}

WigetLayout::~WigetLayout()
{
    //remove wigets
}

void WigetLayout::update(WigetHandler* handler, f32 dt)
{
    for (auto wiget = m_wigets.begin(); wiget != m_wigets.end(); ++wiget)
    {
        if ((*wiget)->isVisible())
        {
            (*wiget)->update(handler, this, dt);
        }
    }
}

} //namespace scene
} //namespace v3d