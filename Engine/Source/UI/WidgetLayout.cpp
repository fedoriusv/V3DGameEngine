#include "WidgetLayout.h"
#include "WidgetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{

namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetLayout::WidgetLayout(LayoutFlags flags) noexcept
    : WidgetLayoutBase<WidgetLayout>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    Widget::cast_data<StateType>(m_data)._flags = flags;
}

WidgetLayout::WidgetLayout(const WidgetLayout& other) noexcept
    : WidgetLayoutBase<WidgetLayout>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;

    for (const Widget* wiget : other.m_wigets)
    {
        m_wigets.push_back(wiget->copy());
    }
}

WidgetLayout::WidgetLayout(WidgetLayout&& other) noexcept
    : WidgetLayoutBase<WidgetLayout>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;

    m_wigets = std::move(other.m_wigets);
}

WidgetLayout::~WidgetLayout()
{
    for (Widget* wiget : m_wigets)
    {
        V3D_DELETE(wiget, memory::MemoryLabel::MemoryUI);
    }
    m_wigets.clear();

    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

TypePtr WidgetLayout::getType() const
{
    return typeOf<WidgetLayout>();
}

bool WidgetLayout::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    ///
    if (cast_data<StateType>(m_data)._flags & WidgetLayout::LayoutFlag::Test)
    {
        int a = 0;
    }
    ///

    math::float2 layoutSize = { 0.0f , 0.0f };
    u32 countWidgets = 0;
    f32 horizontLineHeight = 0;
    for (auto wigetIter = m_wigets.begin(); wigetIter != m_wigets.end(); ++wigetIter)
    {
        Widget* widget = *wigetIter;
        if (widget->isVisible())
        {
            math::float2 size = widget->calculateSize(handler, parent, this);
            if (cast_data<StateType>(m_data)._stateMask & Widget::State::StateMask::HorizontalLine)
            {
                layoutSize._x += size._x;
                layoutSize._y = std::max<f32>(size._y, layoutSize._y);
            }
            else
            {
                //layoutSize._x = std::max(layoutSize._x, size._x);
                layoutSize._y += size._y;
            }
            ++countWidgets;
        }
    }

    WidgetDrawer* drawer = handler->getWidgetDrawer();
    if (cast_data<StateType>(m_data)._stateMask & Widget::State::StateMask::HorizontalLine)
    {
        layoutSize._x = 0;
        layoutSize._y += drawer->get_LayoutPadding()._y * 2.0f;
    }
    else
    {
        layoutSize._y += drawer->get_WindowPadding()._y * 2.0f;
        layoutSize._y += drawer->get_LayoutPadding()._y * 2.0f;
        layoutSize._y += drawer->get_ItemSpacing()._y * std::clamp<u32>(countWidgets - 1, 0, countWidgets);
    }

    drawer->draw_BeginLayoutState(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, layoutSize);

    switch (getVAlignment())
    {
    case VerticalAlignment::AlignmentTop:
    case VerticalAlignment::AlignmentCenter:
    default:
    {
        for (auto wiget = m_wigets.begin(); wiget != m_wigets.end(); ++wiget)
        {
            if ((*wiget)->isVisible())
            {
                (*wiget)->update(handler, parent, this, dt);
            }
        }
        break;
    }

    case VerticalAlignment::AlignmentBottom:
    {
        for (auto wiget = m_wigets.rbegin(); wiget != m_wigets.rend(); ++wiget)
        {
            if ((*wiget)->isVisible())
            {
                (*wiget)->update(handler, parent, this, dt);
            }
        }
        break;
    }
    }

    drawer->draw_EndLayoutState(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data, layoutSize);

    return true;
}

Widget* WidgetLayout::copy() const
{
    return V3D_NEW(WidgetLayout, memory::MemoryLabel::MemoryUI)(*this);
}

math::float2 WidgetLayout::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    math::float2 layoutSize = { 0.0f , 0.0f };
    u32 countWidgets = 0;
    for (auto wigetIter = m_wigets.begin(); wigetIter != m_wigets.end(); ++wigetIter)
    {
        Widget* widget = *wigetIter;
        if (widget->isVisible())
        {
            math::float2 size = widget->calculateSize(handler, parent, this);
            if (cast_data<StateType>(m_data)._stateMask & Widget::State::StateMask::HorizontalLine)
            {
                layoutSize._x += size._x;
                layoutSize._y = std::max<f32>(size._y, layoutSize._y);
            }
            else
            {
                //layoutSize._x = std::max(layoutSize._x, size._x);
                layoutSize._y += size._y;
            }
            ++countWidgets;
        }
    }

    WidgetDrawer* drawer = handler->getWidgetDrawer();
    if (cast_data<StateType>(m_data)._stateMask & Widget::State::StateMask::HorizontalLine)
    {
        layoutSize._x = 0;
        layoutSize._y += drawer->get_LayoutPadding()._y * 2.0f;
    }
    else
    {
        layoutSize._y += drawer->get_LayoutPadding()._y * 2.0f;
        layoutSize._y += drawer->get_ItemSpacing()._y * std::clamp<u32>(countWidgets, 0, countWidgets);
    }

    return layoutSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetHorizontalLayout::WidgetHorizontalLayout() noexcept
    : WidgetLayout()
{
    Widget::cast_data<StateType>(m_data)._stateMask |= State::StateMask::HorizontalLine;
}

WidgetHorizontalLayout::WidgetHorizontalLayout(const WidgetHorizontalLayout& other) noexcept
    : WidgetLayout(other)
{
}

WidgetHorizontalLayout::WidgetHorizontalLayout(WidgetHorizontalLayout&& other) noexcept
    : WidgetLayout(other)
{
}

WidgetHorizontalLayout::~WidgetHorizontalLayout()
{
}

TypePtr WidgetHorizontalLayout::getType() const
{
    return typeOf<WidgetHorizontalLayout>();
}

Widget* WidgetHorizontalLayout::copy() const
{
    return V3D_NEW(WidgetHorizontalLayout, memory::MemoryLabel::MemoryUI)(*this);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetWindowLayout::WidgetWindowLayout() noexcept
    : WidgetBase<WidgetWindowLayout>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetWindowLayout::WidgetWindowLayout(WidgetWindow* main, const std::vector<LayoutRule>& layouts) noexcept
    : WidgetBase<WidgetWindowLayout>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    Widget::cast_data<StateType>(m_data)._main = main;
    Widget::cast_data<StateType>(m_data)._rules = layouts;
}

WidgetWindowLayout::WidgetWindowLayout(const WidgetWindowLayout& other) noexcept
    : WidgetBase<WidgetWindowLayout>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetWindowLayout::WidgetWindowLayout(WidgetWindowLayout&& other) noexcept
    : WidgetBase<WidgetWindowLayout>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetWindowLayout::~WidgetWindowLayout()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

WidgetWindowLayout& WidgetWindowLayout::operator=(const WidgetWindowLayout& other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;

    return *this;
}

WidgetWindowLayout& WidgetWindowLayout::operator=(WidgetWindowLayout&& other)
{
    m_data = other.m_data;
    other.m_data = nullptr;

    return *this;
}

TypePtr WidgetWindowLayout::getType() const
{
    return typeOf<WidgetWindowLayout>();
}

bool WidgetWindowLayout::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    return false;
}

Widget* WidgetWindowLayout::copy() const
{
    return V3D_NEW(WidgetWindowLayout, memory::MemoryLabel::MemoryUI)(*this);
}

} //namespace scene
} //namespace v3d