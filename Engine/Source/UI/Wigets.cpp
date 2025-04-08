#include "Wigets.h"
#include "WigetHandler.h"

namespace v3d
{
namespace ui
{

WigetButton::WigetButton(const std::string& title) noexcept
    : WigetBase<WigetButton>(V3D_NEW(WigetButton::ContextButton, memory::MemoryLabel::MemoryUI)())
{
    setTitle(title);
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
    ContextButton& context = Wiget::cast_data<ContextButton>(m_data);
    if (context._onUpdate)
    {
        std::invoke(context._onUpdate, this, dt);
    }

    return handler->draw_Button(this, m_data, dt);
}



WigetImage::WigetImage() noexcept
    : WigetBase<WigetImage>(V3D_NEW(WigetImage::ContextImage, memory::MemoryLabel::MemoryUI)())
{
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
    ContextImage& context = Wiget::cast_data<ContextImage>(m_data);
    if (context._onUpdate)
    {
        std::invoke(context._onUpdate, this, dt);
    }

    return handler->draw_Button(this, m_data, dt);
}

} // namespace ui
} // namespace v3d