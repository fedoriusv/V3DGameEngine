#pragma once

#include "Widget.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetMenuBar wiget class
    */
    class WidgetMenuBar final : public WidgetBase<WidgetMenuBar>
    {
    public:

        enum MenuFlag
        {
            MainMenu = 1 << 0,
        };
        typedef u64 MenuFlags;

        WidgetMenuBar(MenuFlags flags = 0) noexcept;
        WidgetMenuBar(const WidgetMenuBar&) noexcept;
        WidgetMenuBar(WidgetMenuBar&&) noexcept;
        ~WidgetMenuBar();

        template<class TWidget>
        WidgetMenuBar& addWidget(const TWidget& wiget);

        template<class TWidget>
        WidgetMenuBar& addWidget(TWidget&& wiget);

        TypePtr getType() const final;

        struct StateMenuBar : StateBase
        {
            WidgetLayout _layout;
            MenuFlags   _flags;
        };

    private:

        using WidgetType = WidgetMenuBar;
        using StateType = StateMenuBar;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    template<class TWidget>
    inline WidgetMenuBar& WidgetMenuBar::addWidget(const TWidget& wiget)
    {
        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget<TWidget>(wiget);
        return *this;
    }

    template<class TWidget>
    inline WidgetMenuBar& WidgetMenuBar::addWidget(TWidget&& wiget)
    {
        static_assert(std::is_move_constructible<TWidget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWidget>::value, "should be noexcept movable");

        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget(std::forward<TWidget>(wiget));
        return *this;
    }

    inline TypePtr WidgetMenuBar::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetMenu wiget class
    */
    class WidgetMenu final : public WidgetBase<WidgetMenu>
    {
    public:

        WidgetMenu(const std::string& text) noexcept;
        WidgetMenu(const WidgetMenu&) noexcept;
        WidgetMenu(WidgetMenu&&) noexcept;
        ~WidgetMenu();

        const std::string& getText() const;

        WidgetMenu& setText(const std::string& text);
        WidgetMenu& setOnClickedEvent(const OnWidgetEvent& event);

        template<class TWidget>
        WidgetMenu& addWidget(const TWidget& wiget);

        template<class TWidget>
        WidgetMenu& addWidget(TWidget&& wiget);

        TypePtr getType() const final;

        struct StateMenu : StateBase
        {
            std::string   _text;
            OnWidgetEvent _onClickedEvent;
            WidgetLayout  _layout;
        };

    private:

        using WidgetType = WidgetMenu;
        using StateType = StateMenu;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline const std::string& WidgetMenu::getText() const
    {
        return  Widget::cast_data<StateType>(m_data)._text;
    }

    inline WidgetMenu& WidgetMenu::setText(const std::string& text)
    {
        Widget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WidgetMenu& WidgetMenu::setOnClickedEvent(const OnWidgetEvent& event)
    {
        Widget::cast_data<StateType>(m_data)._onClickedEvent = event;
        return *this;
    }

    inline TypePtr WidgetMenu::getType() const
    {
        return typeOf<WidgetType>();
    }

    template<class TWidget>
    inline WidgetMenu& WidgetMenu::addWidget(const TWidget& wiget)
    {
        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget<TWidget>(wiget);
        return *this;
    }

    template<class TWidget>
    inline WidgetMenu& WidgetMenu::addWidget(TWidget&& wiget)
    {
        static_assert(std::is_move_constructible<TWidget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWidget>::value, "should be noexcept movable");

        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget(std::forward<TWidget>(wiget));
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WidgetMenuItem final : public WidgetBase<WidgetMenuItem>
    {
    public:

        explicit WidgetMenuItem(const std::string& text) noexcept;
        WidgetMenuItem(const WidgetMenuItem&) noexcept;
        WidgetMenuItem(WidgetMenuItem&&) noexcept;
        ~WidgetMenuItem();

        const std::string& getText() const;

        WidgetMenuItem& setText(const std::string& text);
        WidgetMenuItem& setOnClickedEvent(const OnWidgetEvent& event);

        TypePtr getType() const final;

        struct StateMenuItem : StateBase
        {
            std::string  _text;
            OnWidgetEvent _onClickedEvent;
        };

    private:

        using WidgetType = WidgetMenuItem;
        using StateType = StateMenuItem;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline const std::string& WidgetMenuItem::getText() const
    {
        return  Widget::cast_data<StateType>(m_data)._text;
    }

    inline WidgetMenuItem& WidgetMenuItem::setText(const std::string& text)
    {
        Widget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WidgetMenuItem& WidgetMenuItem::setOnClickedEvent(const OnWidgetEvent& event)
    {
        Widget::cast_data<StateType>(m_data)._onClickedEvent = event;
        return *this;
    }

    inline TypePtr WidgetMenuItem::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetMenuBar>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WidgetMenu>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WidgetMenuItem>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d