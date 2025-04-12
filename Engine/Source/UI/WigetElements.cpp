#include "WigetElements.h"
#include "WigetHandler.h"

namespace v3d
{
namespace ui
{

WigetButton::WigetButton(const std::string& text) noexcept
    : WigetBase<WigetButton>(V3D_NEW(WigetButton::ContextButton, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
}

WigetButton::WigetButton(const WigetButton& other) noexcept
    : WigetBase<WigetButton>(other)
{
    WigetButton::ContextButton* context = V3D_NEW(WigetButton::ContextButton, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetButton::ContextButton*>(other.m_data);
    m_data = context;
}

WigetButton::~WigetButton()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetButton::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_Button(this, m_data, dt);
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetImage::WigetImage(const math::Dimension2D& size, const math::RectF32& uv) noexcept
    : WigetBase<WigetImage>(V3D_NEW(WigetImage::ContextImage, memory::MemoryLabel::MemoryUI)())
{
    setSize(size);
    setUVs(uv);
}

WigetImage::WigetImage(const WigetImage& other) noexcept
    : WigetBase<WigetImage>(other)
{
    WigetImage::ContextImage* context = V3D_NEW(WigetImage::ContextImage, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetImage::ContextImage*>(other.m_data);
    m_data = context;
}

WigetImage::~WigetImage()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetImage::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_Image(this, m_data, dt);
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetCheckBox::WigetCheckBox(const std::string& text, bool value) noexcept
    : WigetBase<WigetCheckBox>(V3D_NEW(WigetCheckBox::ContextCheckBox, memory::MemoryLabel::MemoryUI)())
{
    setText(text);
    setValue(value);
}

WigetCheckBox::WigetCheckBox(const WigetCheckBox& other) noexcept
    : WigetBase<WigetCheckBox>(other)
{
    WigetCheckBox::ContextCheckBox* context = V3D_NEW(WigetCheckBox::ContextCheckBox, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetCheckBox::ContextCheckBox*>(other.m_data);
    m_data = context;
}

WigetCheckBox::~WigetCheckBox()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetCheckBox::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    if (Wiget::update(handler, layout, dt))
    {
        return handler->draw_CheckBox(this, m_data, dt);
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetInputField::WigetInputField() noexcept
    : WigetBase<WigetInputField>(V3D_NEW(WigetInputField::ContextInputField, memory::MemoryLabel::MemoryUI)())
{
}

WigetInputField::WigetInputField(const WigetInputField& other) noexcept
    : WigetBase<WigetInputField>(other)
{
    WigetInputField::ContextInputField* context = V3D_NEW(WigetInputField::ContextInputField, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetInputField::ContextInputField*>(other.m_data);
    m_data = context;
}

WigetInputField::~WigetInputField()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetInputField::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    return handler->draw_InputField(this, m_data, dt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetInputSlider::WigetInputSlider() noexcept
    : WigetBase<WigetInputSlider>(V3D_NEW(WigetInputSlider::ContextInputSlider, memory::MemoryLabel::MemoryUI)())
{
}

WigetInputSlider::WigetInputSlider(const WigetInputSlider& other) noexcept
    : WigetBase<WigetInputSlider>(other)
{
    WigetInputSlider::ContextInputSlider* context = V3D_NEW(WigetInputSlider::ContextInputSlider, memory::MemoryLabel::MemoryUI)();
    *context = *static_cast<WigetInputSlider::ContextInputSlider*>(other.m_data);
    m_data = context;
}

WigetInputSlider::~WigetInputSlider()
{
    if (m_data)
    {
        V3D_DELETE(m_data, memory::MemoryLabel::MemoryUI);
        m_data = nullptr;
    }
}

bool WigetInputSlider::update(WigetHandler* handler, WigetLayout* layout, f32 dt)
{
    return handler->draw_InputSlider(this, m_data, dt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d