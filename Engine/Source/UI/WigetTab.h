#pragma once

#include "Wiget.h"
#include "WigetLayout.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetTabBar wiget class
    */
    class WigetTabBar final : public WigetBase<WigetTabBar>
    {
    public:

        WigetTabBar() noexcept;
        WigetTabBar(const WigetTabBar&) noexcept;
        ~WigetTabBar();

        template<class TWiget>
        WigetTabBar& addWiget(const TWiget& wiget);

        struct StateTabBar : StateBase
        {
            WigetLayout _layout;
        };

    private:

        using WigetType = WigetTabBar;
        using StateType = StateTabBar;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    template<class TWiget>
    inline WigetTabBar& WigetTabBar::addWiget(const TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget<TWiget>(wiget);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetTab wiget class
    */
    class WigetTabItem final : public WigetBase<WigetTabItem>
    {
    public:

        WigetTabItem(const std::string& text) noexcept;
        WigetTabItem(const WigetTabItem&) noexcept;
        ~WigetTabItem();

        const std::string& getText() const;

        WigetTabItem& setText(const std::string& text);
        WigetTabItem& setOnClickedEvent(const OnWigetEvent& event);

        template<class TWiget>
        WigetTabItem& addWiget(const TWiget& wiget);

        template<class TWiget>
        WigetTabItem& addWiget(TWiget& wiget);

        struct StateTabItem : StateBase
        {
            std::string  _text;
            OnWigetEvent _onClickedEvent;
            WigetLayout  _layout;
        };

    private:

        using WigetType = WigetTabItem;
        using StateType = StateTabItem;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline const std::string& WigetTabItem::getText() const
    {
        return  Wiget::cast_data<StateType>(m_data)._text;
    }

    inline WigetTabItem& WigetTabItem::setText(const std::string& text)
    {
        Wiget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WigetTabItem& WigetTabItem::setOnClickedEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<StateType>(m_data)._onClickedEvent = event;
        return *this;
    }

    template<class TWiget>
    inline WigetTabItem& WigetTabItem::addWiget(const TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget<TWiget>(wiget);
        return *this;
    }

    template<class TWiget>
    inline WigetTabItem& WigetTabItem::addWiget(TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget<TWiget>(wiget);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d