#pragma once

#include "Wiget.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetMenuBar wiget class
    */
    class WigetMenuBar final : public WigetBase<WigetMenuBar>
    {
    public:

        WigetMenuBar() noexcept;
        WigetMenuBar(const WigetMenuBar&) noexcept;
        ~WigetMenuBar();

        template<class UWiget>
        WigetMenuBar& addWiget(const UWiget& wiget);

        struct ContextMenuBar : ContextBase
        {
            WigetLayout _layout;
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    template<class TWiget>
    inline WigetMenuBar& WigetMenuBar::addWiget(const TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<ContextMenuBar>(m_data)._layout;
        layout.addWiget<TWiget>(wiget);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetMenu wiget class
    */
    class WigetMenu final : public WigetBase<WigetMenu>
    {
    public:

        WigetMenu(const std::string& text) noexcept;
        WigetMenu(const WigetMenu&) noexcept;
        ~WigetMenu();

        const std::string& getText() const;

        WigetMenu& setText(const std::string& text);
        WigetMenu& setOnClickedEvent(const OnWigetEvent& event);

        template<class TWiget>
        WigetMenu& addWiget(const TWiget& wiget);

        struct ContextMenu : ContextBase
        {
            std::string  _text;
            OnWigetEvent _onClickedEvent;
            WigetLayout  _layout;
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    inline const std::string& WigetMenu::getText() const
    {
        return  Wiget::cast_data<ContextMenu>(m_data)._text;
    }

    inline WigetMenu& WigetMenu::setText(const std::string& text)
    {
        Wiget::cast_data<ContextMenu>(m_data)._text = text;
        return *this;
    }

    inline WigetMenu& WigetMenu::setOnClickedEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<ContextMenu>(m_data)._onClickedEvent = event;
        return *this;
    }

    template<class TWiget>
    inline WigetMenu& WigetMenu::addWiget(const TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<ContextMenu>(m_data)._layout;
        layout.addWiget<TWiget>(wiget);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetMenuItem final : public WigetBase<WigetMenuItem>
    {
    public:

        explicit WigetMenuItem(const std::string& text) noexcept;
        WigetMenuItem(const WigetMenuItem&) noexcept;
        ~WigetMenuItem();

        const std::string& getText() const;

        WigetMenuItem& setText(const std::string& text);
        WigetMenuItem& setOnClickedEvent(const OnWigetEvent& event);

        struct ContextMenuItem : ContextBase
        {
            std::string  _text;
            OnWigetEvent _onClickedEvent;
        };

    private:

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;
    };

    inline const std::string& WigetMenuItem::getText() const
    {
        return  Wiget::cast_data<ContextMenuItem>(m_data)._text;
    }

    inline WigetMenuItem& WigetMenuItem::setText(const std::string& text)
    {
        Wiget::cast_data<ContextMenuItem>(m_data)._text = text;
        return *this;
    }

    inline WigetMenuItem& WigetMenuItem::setOnClickedEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<ContextMenuItem>(m_data)._onClickedEvent = event;
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d