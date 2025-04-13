#include "Wiget.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

Wiget::Wiget(Wiget::Context* context) noexcept
    : m_data(context)
{
}

Wiget::Wiget(const Wiget& other) noexcept
    : m_data(nullptr) //allocate and copy on child constructor
{
}

bool Wiget::update(WigetHandler* handler, Wiget* parent, WigetLayout* layout, f32 dt)
{
    ContextBase& context = cast_data<ContextBase>(m_data);
    if (context._stateMask & 0x01)
    {
        if (context._onActiveChanged)
        {
            std::invoke(context._onActiveChanged, this);
        }
        context._stateMask &= ~0x01;
    }

    if (context._stateMask & 0x02)
    {
        if (context._onVisibleChanged)
        {
            std::invoke(cast_data<ContextBase>(m_data)._onVisibleChanged, this);
        }
        context._stateMask &= ~0x02;
    }

    if (context._onUpdate)
    {
        std::invoke(context._onUpdate, this, dt);
    }

    return true;
}

} // namespace ui
} // namespace v3d