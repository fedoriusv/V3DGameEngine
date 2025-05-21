#include "WigetLayout.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{

namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetLayout::WigetLayout(LayoutFlags flags) noexcept
    : WigetLayoutBase<WigetLayout>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    Wiget::cast_data<StateType>(m_data)._flags = flags;
}

WigetLayout::WigetLayout(const WigetLayout& other) noexcept
    : WigetLayoutBase<WigetLayout>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;

    for (const Wiget* wiget : other.m_wigets)
    {
        m_wigets.push_back(wiget->copy());
    }
}

WigetLayout::WigetLayout(WigetLayout&& other) noexcept
    : WigetLayoutBase<WigetLayout>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;

    m_wigets = std::move(other.m_wigets);
}

WigetLayout::~WigetLayout()
{
    for (Wiget* wiget : m_wigets)
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

TypePtr WigetLayout::getType() const
{
    return type_of<WigetLayout>();
}

bool WigetLayout::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    WigetDrawer* drawer = handler->getWigetDrawer();

    math::TVector2D<f32> layoutSize = { 0.0f , 0.0f };
    u32 countWigets = 0;
    f32 horizontLineHeight = 0;
    for (auto wiget = m_wigets.begin(); wiget != m_wigets.end(); ++wiget)
    {
        if ((*wiget)->isVisible())
        {
            if (cast_data<StateType>(m_data)._stateMask & Wiget::State::StateMask::HorizontalLine)
            {
                math::TVector2D<f32> size = (*wiget)->calculateSize(handler, parent, this);
                layoutSize._x += size._x;
                layoutSize._y = std::max<f32>(size._y, layoutSize._y);
            }
            else
            {
                layoutSize._y += (*wiget)->calculateSize(handler, parent, this)._y;
            }
            ++countWigets;
        }
    }

    if (cast_data<StateType>(m_data)._stateMask & Wiget::State::StateMask::HorizontalLine)
    {
        layoutSize._x = 0;
        layoutSize._y += drawer->get_LayoutPadding()._y * 2.0f;
    }
    else
    {
        layoutSize._y += drawer->get_LayoutPadding()._y * 2.0f;
        layoutSize._y += drawer->get_ItemSpacing()._y * std::clamp<u32>(countWigets - 1, 0, countWigets);
    }

    cast_data<StateType>(m_data)._cachedContentSize = layoutSize;

    drawer->draw_BeginLayoutState(this, parent, m_data);

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

    drawer->draw_EndLayoutState(this, parent, m_data);

    return true;
}

Wiget* WigetLayout::copy() const
{
    return V3D_NEW(WigetLayout, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetHorizontalLayout::WigetHorizontalLayout() noexcept
    : WigetLayout()
{
    Wiget::cast_data<StateType>(m_data)._stateMask |= State::StateMask::HorizontalLine;
}

WigetHorizontalLayout::WigetHorizontalLayout(const WigetHorizontalLayout& other) noexcept
    : WigetLayout(other)
{
}

WigetHorizontalLayout::WigetHorizontalLayout(WigetHorizontalLayout&& other) noexcept
    : WigetLayout(other)
{
}

WigetHorizontalLayout::~WigetHorizontalLayout()
{
}

TypePtr WigetHorizontalLayout::getType() const
{
    return type_of<WigetHorizontalLayout>();
}

Wiget* WigetHorizontalLayout::copy() const
{
    return V3D_NEW(WigetHorizontalLayout, memory::MemoryLabel::MemoryUI)(*this);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetWindowLayout::WigetWindowLayout() noexcept
    : WigetBase<WigetWindowLayout>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WigetWindowLayout::WigetWindowLayout(WigetWindow* main, const std::vector<LayoutRule>& layouts) noexcept
    : WigetBase<WigetWindowLayout>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    Wiget::cast_data<StateType>(m_data)._main = main;
    Wiget::cast_data<StateType>(m_data)._rules = layouts;
}

WigetWindowLayout::WigetWindowLayout(const WigetWindowLayout& other) noexcept
    : WigetBase<WigetWindowLayout>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetWindowLayout::WigetWindowLayout(WigetWindowLayout&& other) noexcept
    : WigetBase<WigetWindowLayout>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetWindowLayout::~WigetWindowLayout()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

WigetWindowLayout& WigetWindowLayout::operator=(const WigetWindowLayout& other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;

    return *this;
}

WigetWindowLayout& WigetWindowLayout::operator=(WigetWindowLayout&& other)
{
    m_data = other.m_data;
    other.m_data = nullptr;

    return *this;
}

TypePtr WigetWindowLayout::getType() const
{
    return type_of<WigetWindowLayout>();
}

bool WigetWindowLayout::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    return false;
}

Wiget* WigetWindowLayout::copy() const
{
    return V3D_NEW(WigetWindowLayout, memory::MemoryLabel::MemoryUI)(*this);
}

} //namespace scene
} //namespace v3d