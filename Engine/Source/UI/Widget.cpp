#include "Widget.h"
#include "WidgetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

static u64 s_uidGenerator = 0;

Widget::Widget(Widget::State* state) noexcept
    : m_data(state)
{
    ++s_uidGenerator;
    cast_data<StateBase>(m_data)._uid = s_uidGenerator;
}

Widget::Widget(const Widget& other) noexcept
    : m_data(nullptr) //Allocate and copy in the child constructor
{
}

Widget::Widget(Widget&& other) noexcept
{
}

Widget::~Widget()
{
    ASSERT(m_data == nullptr, "must be deleted");
}

bool Widget::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    StateBase& state = cast_data<StateBase>(m_data);

    if (!state._isCreated)
    {
        if (state._onCreated)
        {
            std::invoke(state._onCreated, this);
        }
        state._isCreated = true;
    }

    if (state._stateMask & State::StateMask::Active)
    {
        if (state._onActiveChanged)
        {
            std::invoke(state._onActiveChanged, this);
        }
        state._stateMask &= ~State::StateMask::Active;
    }

    if (state._stateMask & State::StateMask::Visible)
    {
        if (state._onVisibleChanged)
        {
            std::invoke(cast_data<StateBase>(m_data)._onVisibleChanged, this);
        }
        state._stateMask &= ~State::StateMask::Visible;
    }

    if (state._onUpdate)
    {
        std::invoke(state._onUpdate, this, dt);
    }

    return true;
}

math::TVector2D<f32> Widget::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    return { 0.0f, 0.0f };
}

void Widget::handleNotify(const utils::Reporter<WidgetReport>* reporter, const WidgetReport& data)
{
}

} // namespace ui
} // namespace v3d