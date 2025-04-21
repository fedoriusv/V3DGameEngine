#include "WigetElements.h"
#include "WigetHandler.h"

namespace v3d
{
namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetText::WigetText(const std::string& text) noexcept
    : WigetBase<WigetText>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WigetText::WigetText(const WigetText& other) noexcept
    : WigetBase<WigetText>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)(*static_cast<const StateType*>(other.m_data));
    m_data = state;
}

WigetText::WigetText(WigetText&& other) noexcept
    : WigetBase<WigetText>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetText::~WigetText()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetText::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_Text(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

math::Vector2D WigetText::calculateSize(WigetHandler* handler, Wiget* parent, Wiget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWigetDrawer()->calculate_TextSize(this, static_cast<WigetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Wiget* WigetText::copy() const
{
    return V3D_NEW(WigetText, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetButton::WigetButton(const std::string& text, const math::Dimension2D& size) noexcept
    : WigetBase<WigetButton>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
    setSize(size);
}

WigetButton::WigetButton(const WigetButton& other) noexcept
    : WigetBase<WigetButton>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WigetButton::WigetButton(WigetButton&& other) noexcept
    : WigetBase<WigetButton>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetButton::~WigetButton()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetButton::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_Button(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

math::Vector2D WigetButton::calculateSize(WigetHandler* handler, Wiget* parent, Wiget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWigetDrawer()->calculate_ButtonSize(this, static_cast<WigetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Wiget* WigetButton::copy() const
{
    return V3D_NEW(WigetButton, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetImage::WigetImage(const renderer::Texture2D* texture, const math::Dimension2D& size, const math::RectF32& uv) noexcept
    : WigetBase<WigetImage>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setTexture(texture);
    setSize(size);
    setUVs(uv);
}

WigetImage::WigetImage(const WigetImage& other) noexcept
    : WigetBase<WigetImage>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WigetImage::WigetImage(WigetImage&& other) noexcept
    : WigetBase<WigetImage>(other)
{
    m_data = other.m_data;
    other.m_data = nullptr;
}

WigetImage::~WigetImage()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetImage::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_Image(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

math::Vector2D WigetImage::calculateSize(WigetHandler* handler, Wiget* parent, Wiget* layout)
{
    m_data->_itemRect = { {0, 0}, handler->getWigetDrawer()->calculate_ImageSize(this, static_cast<WigetType*>(layout)->m_data, m_data) };
    return m_data->_itemRect.getSize();
}

Wiget* WigetImage::copy() const
{
    return V3D_NEW(WigetImage, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetCheckBox::WigetCheckBox(const std::string& text, bool value) noexcept
    : WigetBase<WigetCheckBox>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
    setValue(value);
}

WigetCheckBox::WigetCheckBox(const WigetCheckBox& other) noexcept
    : WigetBase<WigetCheckBox>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WigetCheckBox::~WigetCheckBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetCheckBox::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_CheckBox(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

Wiget* WigetCheckBox::copy() const
{
    return V3D_NEW(WigetCheckBox, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetInputField::WigetInputField() noexcept
    : WigetBase<WigetInputField>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WigetInputField::WigetInputField(const WigetInputField& other) noexcept
    : WigetBase<WigetInputField>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WigetInputField::~WigetInputField()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetInputField::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_InputField(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

Wiget* WigetInputField::copy() const
{
    return V3D_NEW(WigetInputField, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetInputSlider::WigetInputSlider() noexcept
    : WigetBase<WigetInputSlider>(V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)())
{
}

WigetInputSlider::WigetInputSlider(const WigetInputSlider& other) noexcept
    : WigetBase<WigetInputSlider>(other)
{
    StateType* state = V3D_NEW(StateType, memory::MemoryLabel::MemoryUI)();
    *state = *static_cast<StateType*>(other.m_data);
    m_data = state;
}

WigetInputSlider::~WigetInputSlider()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetInputSlider::update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt)
{
    if (Wiget::update(handler, parent, layout, dt))
    {
        return handler->getWigetDrawer()->draw_InputSlider(this, parent, static_cast<WigetType*>(layout)->m_data, m_data);
    }

    return false;
}

Wiget* WigetInputSlider::copy() const
{
    return V3D_NEW(WigetInputSlider, memory::MemoryLabel::MemoryUI)(*this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d