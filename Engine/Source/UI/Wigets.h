#pragma once

#include "Common.h"
#include "Wiget.h"
#include "WigetWindow.h"
#include "WigetMenu.h"
#include "WigetTabBar.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetButton : public WigetBase<WigetButton>
    {
    public:

        explicit WigetButton(const std::string& title) noexcept;
        WigetButton(const WigetButton&) noexcept;
        ~WigetButton();

        const std::string& getTitle() const;

        WigetButton& setTitle(const std::string& title);

        WigetButton& setOnClickedEvent(const OnWigetEvent& event);
        WigetButton& setOnHoveredEvent(const OnWigetEvent& event);

        struct ContextButton : ContextBase
        {
            std::string     _title;
            OnWigetEvent    _onClickedEvent;
            OnWigetEvent    _onPressedEvent;
            OnWigetEvent    _onReleasedEvent;
            OnWigetEvent    _onHoveredEvent;
            OnWigetEvent    _onUnhoveredEvent;
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    inline const std::string& WigetButton::getTitle() const
    {
        return Wiget::cast_data<ContextButton>(m_data)._title;
    }

    inline WigetButton& WigetButton::setTitle(const std::string& title)
    {
        Wiget::cast_data<ContextButton>(m_data)._title = title;
        return *this;
    }

    inline WigetButton& WigetButton::setOnClickedEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<ContextButton>(m_data)._onClickedEvent = event;
        return *this;
    }

    inline WigetButton& WigetButton::setOnHoveredEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<ContextButton>(m_data)._onHoveredEvent = event;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetImage : public WigetBase<WigetImage>
    {
    public:

        WigetImage() noexcept;
        WigetImage(const WigetImage&) noexcept;
        ~WigetImage();

        struct ContextImage : ContextBase
        {
            OnWigetEvent    _onClickedEvent;
            OnWigetEvent    _onPressedEvent;
            OnWigetEvent    _onReleasedEvent;
            OnWigetEvent    _onHoveredEvent;
            OnWigetEvent    _onUnhoveredEvent;
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d