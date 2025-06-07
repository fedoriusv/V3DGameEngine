#pragma once

#include "Widget.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetWindow wiget class
    */
    class WidgetWindow final : public WidgetBase<WidgetWindow>
    {
    public:

        enum WindowFlag
        {
            Moveable = 1 << 0,
            Resizeable = 1 << 1,
            Scrollable = 1 << 2,
            AutoResizeByContent = 1 << 3,
        };
        typedef u64 WindowFlags;

        explicit WidgetWindow(const std::string& title, WindowFlags flags = 0) noexcept;
        explicit WidgetWindow(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos, WindowFlags flags = 0) noexcept;
        WidgetWindow(const WidgetWindow&) noexcept;
        WidgetWindow(WidgetWindow&&) noexcept;
        ~WidgetWindow();

        bool isFocused() const;

        const math::Dimension2D& getSize() const;
        const math::Point2D& getPosition() const;
        const std::string& getTitle() const;

        const platform::Window* getWindow() const;

        WidgetWindow& setSize(const math::Dimension2D& size);
        WidgetWindow& setPosition(const math::Point2D& position);
        WidgetWindow& setTitle(const std::string& title);

        WidgetWindow& setOnSizeChanged(const OnWidgetEventDimention2DParam& event);
        WidgetWindow& setOnPositionChanged(const OnWidgetEventPoint2DParam& event);
        WidgetWindow& setOnFocusChanged(const OnWidgetEventBoolParam& event);
        WidgetWindow& setOnClosed(const OnWidgetEvent& event);

        template<class TWidget>
        WidgetWindow& addWidget(const TWidget& wiget);

        template<class TWidget>
        WidgetWindow& addWidget(TWidget&& wiget);

        Widget* findWidgetByID(u64 id);

        WidgetWindow& setupWindowLayout(const WidgetWindowLayout& layout);
        WidgetWindow& setupWindowLayout(WidgetWindowLayout&& layout);

        TypePtr getType() const final;

        struct StateWindow : StateBase
        {
            math::Dimension2D             _size;
            math::Point2D                 _position;
            std::string                   _title;
            WindowFlags                   _createFlags = 0;
            WidgetLayout                  _layout;
            WidgetWindowLayout            _windowLayout;
            OnWidgetEventDimention2DParam _onSizeChanged;
            OnWidgetEventPoint2DParam     _onPositionChanged;
            OnWidgetEventBoolParam        _onFocusChanged;
            OnWidgetEvent                 _onClosed;

            //States
            platform::Window*             _currentWindow = nullptr;
            math::Rect                    _cachedWindowRect;
            math::TVector2D<f32>          _cachedWindowOffest;
        };

    private:

        using WidgetType = WidgetWindow;
        using StateType = StateWindow;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline bool WidgetWindow::isFocused() const
    {
        return Widget::cast_data<StateType>(m_data)._isFocused;
    }

    inline const math::Dimension2D& WidgetWindow::getSize() const
    {
        return Widget::cast_data<StateType>(m_data)._size;
    }

    inline const math::Point2D& WidgetWindow::getPosition() const
    {
        return  Widget::cast_data<StateType>(m_data)._position;
    }

    inline const std::string& WidgetWindow::getTitle() const
    {
        return  Widget::cast_data<StateType>(m_data)._title;
    }

    inline const platform::Window* WidgetWindow::getWindow() const
    {
        return  Widget::cast_data<StateType>(m_data)._currentWindow;
    }

    inline WidgetWindow& WidgetWindow::setSize(const math::Dimension2D& size)
    {
        Widget::cast_data<StateType>(m_data)._size = size;
        return *this;
    }

    inline WidgetWindow& WidgetWindow::setPosition(const math::Point2D& position)
    {
        Widget::cast_data<StateType>(m_data)._position = position;
        return *this;
    }

    inline WidgetWindow& WidgetWindow::setTitle(const std::string& title)
    {
        Widget::cast_data<StateType>(m_data)._title = title;
        return *this;
    }

    inline WidgetWindow& WidgetWindow::setOnSizeChanged(const OnWidgetEventDimention2DParam& event)
    {
        cast_data<StateType>(m_data)._onSizeChanged = event;
        return *this;
    }

    inline WidgetWindow& WidgetWindow::setOnPositionChanged(const OnWidgetEventPoint2DParam& event)
    {
        cast_data<StateType>(m_data)._onPositionChanged = event;
        return *this;
    }

    inline WidgetWindow& WidgetWindow::setOnFocusChanged(const OnWidgetEventBoolParam& event)
    {
        cast_data<StateType>(m_data)._onFocusChanged = event;
        return *this;
    }

    inline WidgetWindow& WidgetWindow::setOnClosed(const OnWidgetEvent& event)
    {
        cast_data<StateType>(m_data)._onClosed = event;
        return *this;
    }

    template<class TWidget>
    inline WidgetWindow& WidgetWindow::addWidget(const TWidget& wiget)
    {
        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget(wiget);
        return *this;
    }

    template<class TWidget>
    inline WidgetWindow& WidgetWindow::addWidget(TWidget&& wiget)
    {
        static_assert(std::is_move_constructible<TWidget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWidget>::value, "should be noexcept movable");

        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget(std::forward<TWidget>(wiget));
        return *this;
    }

    inline WidgetWindow& WidgetWindow::setupWindowLayout(const WidgetWindowLayout& layout)
    {
        cast_data<StateType>(m_data)._windowLayout = layout;
        return *this;
    }

    inline WidgetWindow& WidgetWindow::setupWindowLayout(WidgetWindowLayout&& layout)
    {
        cast_data<StateType>(m_data)._windowLayout = std::forward<WidgetWindowLayout>(layout);
        return *this;
    }

    inline TypePtr WidgetWindow::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetWindow>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d