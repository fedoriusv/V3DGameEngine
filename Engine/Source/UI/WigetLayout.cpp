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

void WigetLayout::update(WigetHandler* handler, Wiget* parent, f32 dt)
{
    for (auto wiget = m_wigets.begin(); wiget != m_wigets.end(); ++wiget)
    {
        if ((*wiget)->isVisible())
        {
            (*wiget)->update(handler, parent, this, dt);
        }
    }
}

} //namespace scene
} //namespace v3d