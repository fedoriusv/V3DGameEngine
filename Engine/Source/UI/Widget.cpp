#include "Widget.h"
#include "WidgetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

static u64 s_uidGenerator = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////

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

math::float2 Widget::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    return { 0.0f, 0.0f };
}

void Widget::handleNotify(const utils::Reporter<WidgetReport>* reporter, const WidgetReport& data)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetTEST::WidgetTEST() noexcept
    : WidgetBase<WidgetType>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetTEST::WidgetTEST(const WidgetTEST& other) noexcept
    : WidgetBase<WidgetType>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetTEST::WidgetTEST(WidgetTEST&& other) noexcept
    : WidgetBase<WidgetType>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetTEST::~WidgetTEST()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetTEST::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_TEST(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

Widget* WidgetTEST::copy() const
{
    return V3D_NEW(WidgetTEST, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d