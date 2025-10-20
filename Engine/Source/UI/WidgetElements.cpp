#include "WidgetElements.h"
#include "WidgetHandler.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetText::WidgetText(const std::string& text) noexcept
    : WidgetBase<WidgetText>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WidgetText::WidgetText(const WidgetText& other) noexcept
    : WidgetBase<WidgetText>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WidgetText::WidgetText(WidgetText&& other) noexcept
    : WidgetBase<WidgetText>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetText::~WidgetText()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetText::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_Text(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

math::float2 WidgetText::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWidgetDrawer()->calculate_TextSize(this, static_cast<WidgetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Widget* WidgetText::copy() const
{
    return V3D_NEW(WidgetText, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetButton::WidgetButton(const std::string& text) noexcept
    : WidgetBase<WidgetButton>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WidgetButton::WidgetButton(const WidgetButton& other) noexcept
    : WidgetBase<WidgetButton>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetButton::WidgetButton(WidgetButton&& other) noexcept
    : WidgetBase<WidgetButton>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetButton::~WidgetButton()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetButton::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_Button(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

math::float2 WidgetButton::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWidgetDrawer()->calculate_ButtonSize(this, static_cast<WidgetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Widget* WidgetButton::copy() const
{
    return V3D_NEW(WidgetButton, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetImage::WidgetImage(renderer::Texture2D* texture, const math::Dimension2D& size, const math::Rect& uv) noexcept
    : WidgetBase<WidgetImage>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setTexture(texture);
    setSize(size);
    setUVs(uv);
}

WidgetImage::WidgetImage(const WidgetImage& other) noexcept
    : WidgetBase<WidgetImage>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetImage::WidgetImage(WidgetImage&& other) noexcept
    : WidgetBase<WidgetImage>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetImage::~WidgetImage()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetImage::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_Image(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

math::float2 WidgetImage::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWidgetDrawer()->calculate_ImageSize(this, static_cast<WidgetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Widget* WidgetImage::copy() const
{
    return V3D_NEW(WidgetImage, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetCheckBox::WidgetCheckBox(const std::string& text, bool value) noexcept
    : WidgetBase<WidgetCheckBox>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
    setValue(value);
}

WidgetCheckBox::WidgetCheckBox(const WidgetCheckBox& other) noexcept
    : WidgetBase<WidgetCheckBox>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetCheckBox::WidgetCheckBox(WidgetCheckBox&& other) noexcept
    : WidgetBase<WidgetCheckBox>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WidgetCheckBox::~WidgetCheckBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetCheckBox::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_CheckBox(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

math::float2 WidgetCheckBox::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWidgetDrawer()->calculate_CheckBoxSize(this, static_cast<WidgetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Widget* WidgetCheckBox::copy() const
{
    return V3D_NEW(WidgetCheckBox, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetSeparator::WidgetSeparator() noexcept
    : WidgetBase<WidgetSeparator>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WidgetSeparator::WidgetSeparator(const WidgetSeparator& other) noexcept
    : WidgetBase<WidgetSeparator>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WidgetSeparator::~WidgetSeparator()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WidgetSeparator::update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt)
{
    if (Widget::update(handler, parent, layout, dt))
    {
        return handler->getWidgetDrawer()->draw_Separator(this, parent, static_cast<WidgetType*>(layout)->m_data, m_data);
    }

    return false;
}

math::float2 WidgetSeparator::calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout)
{
    return math::float2();
}

Widget* WidgetSeparator::copy() const
{
    return V3D_NEW(WidgetSeparator, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d