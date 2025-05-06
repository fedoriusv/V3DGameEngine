#pragma once

#include "Wiget.h"
#include "WigetLayout.h"

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

        enum MenuFlag
        {
            MainMenu = 1 << 0,
        };
        typedef u64 MenuFlags;

        WigetMenuBar(MenuFlags flags = 0) noexcept;
        WigetMenuBar(const WigetMenuBar&) noexcept;
        WigetMenuBar(WigetMenuBar&&) noexcept;
        ~WigetMenuBar();

        template<class TWiget>
        WigetMenuBar& addWiget(const TWiget& wiget);

        template<class TWiget>
        WigetMenuBar& addWiget(TWiget&& wiget);

        TypePtr getType() const final;

        struct StateMenuBar : StateBase
        {
            WigetLayout _layout;
            MenuFlags   _flags;
        };

    private:

        using WigetType = WigetMenuBar;
        using StateType = StateMenuBar;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    template<class TWiget>
    inline WigetMenuBar& WigetMenuBar::addWiget(const TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget<TWiget>(wiget);
        return *this;
    }

    template<class TWiget>
    inline WigetMenuBar& WigetMenuBar::addWiget(TWiget&& wiget)
    {
        static_assert(std::is_move_constructible<TWiget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWiget>::value, "should be noexcept movable");

        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget(std::forward<TWiget>(wiget));
        return *this;
    }

    inline TypePtr WigetMenuBar::getType() const
    {
        return type_of<WigetType>();
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
        WigetMenu(WigetMenu&&) noexcept;
        ~WigetMenu();

        const std::string& getText() const;

        WigetMenu& setText(const std::string& text);
        WigetMenu& setOnClickedEvent(const OnWigetEvent& event);

        template<class TWiget>
        WigetMenu& addWiget(const TWiget& wiget);

        template<class TWiget>
        WigetMenu& addWiget(TWiget&& wiget);

        TypePtr getType() const final;

        struct StateMenu : StateBase
        {
            std::string  _text;
            OnWigetEvent _onClickedEvent;
            WigetLayout  _layout;
        };

    private:

        using WigetType = WigetMenu;
        using StateType = StateMenu;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline const std::string& WigetMenu::getText() const
    {
        return  Wiget::cast_data<StateType>(m_data)._text;
    }

    inline WigetMenu& WigetMenu::setText(const std::string& text)
    {
        Wiget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WigetMenu& WigetMenu::setOnClickedEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<StateType>(m_data)._onClickedEvent = event;
        return *this;
    }

    inline TypePtr WigetMenu::getType() const
    {
        return type_of<WigetType>();
    }

    template<class TWiget>
    inline WigetMenu& WigetMenu::addWiget(const TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget<TWiget>(wiget);
        return *this;
    }

    template<class TWiget>
    inline WigetMenu& WigetMenu::addWiget(TWiget&& wiget)
    {
        static_assert(std::is_move_constructible<TWiget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWiget>::value, "should be noexcept movable");

        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget(std::forward<TWiget>(wiget));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetMenuItem final : public WigetBase<WigetMenuItem>
    {
    public:

        explicit WigetMenuItem(const std::string& text) noexcept;
        WigetMenuItem(const WigetMenuItem&) noexcept;
        WigetMenuItem(WigetMenuItem&&) noexcept;
        ~WigetMenuItem();

        const std::string& getText() const;

        WigetMenuItem& setText(const std::string& text);
        WigetMenuItem& setOnClickedEvent(const OnWigetEvent& event);

        TypePtr getType() const final;

        struct StateMenuItem : StateBase
        {
            std::string  _text;
            OnWigetEvent _onClickedEvent;
        };

    private:

        using WigetType = WigetMenuItem;
        using StateType = StateMenuItem;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline const std::string& WigetMenuItem::getText() const
    {
        return  Wiget::cast_data<StateType>(m_data)._text;
    }

    inline WigetMenuItem& WigetMenuItem::setText(const std::string& text)
    {
        Wiget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WigetMenuItem& WigetMenuItem::setOnClickedEvent(const OnWigetEvent& event)
    {
        Wiget::cast_data<StateType>(m_data)._onClickedEvent = event;
        return *this;
    }

    inline TypePtr WigetMenuItem::getType() const
    {
        return type_of<WigetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WigetMenuBar>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WigetMenu>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WigetMenuItem>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d