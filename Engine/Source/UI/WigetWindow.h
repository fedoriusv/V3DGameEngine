#pragma once

#include "Wiget.h"
#include "WigetLayout.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetWindow wiget class
    */
    class WigetWindow final : public WigetBase<WigetWindow>
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

        explicit WigetWindow(const std::string& title, WindowFlags flags = 0) noexcept;
        explicit WigetWindow(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos, WindowFlags flags = 0) noexcept;
        WigetWindow(const WigetWindow&) noexcept;
        WigetWindow(WigetWindow&&) noexcept;
        ~WigetWindow();

        bool isFocused() const;

        const math::Dimension2D& getSize() const;
        const math::Point2D& getPosition() const;
        const std::string& getTitle() const;

        const platform::Window* getWindow() const;

        WigetWindow& setSize(const math::Dimension2D& size);
        WigetWindow& setPosition(const math::Point2D& position);
        WigetWindow& setTitle(const std::string& title);

        WigetWindow& setOnSizeChanged(const OnWigetEventDimention2DParam& event);
        WigetWindow& setOnPositionChanged(const OnWigetEventPoint2DParam& event);
        WigetWindow& setOnFocusChanged(const OnWigetEventBoolParam& event);
        WigetWindow& setOnClosed(const OnWigetEvent& event);

        template<class TWiget>
        WigetWindow& addWiget(const TWiget& wiget);

        template<class TWiget>
        WigetWindow& addWiget(TWiget&& wiget);

        Wiget* findWidgetByID(u64 id);

        WigetWindow& setupWindowLayout(const WigetWindowLayout& layout);
        WigetWindow& setupWindowLayout(WigetWindowLayout&& layout);

        TypePtr getType() const final;

        struct StateWindow : StateBase
        {
            math::Dimension2D               _size;
            math::Point2D                   _position;
            std::string                     _title;
            WindowFlags                     _createFlags = 0;
            WigetLayout                     _layout;
            WigetWindowLayout               _windowLayout;
            OnWigetEventDimention2DParam    _onSizeChanged;
            OnWigetEventPoint2DParam        _onPositionChanged;
            OnWigetEventBoolParam           _onFocusChanged;
            OnWigetEvent                    _onClosed;

            //States
            platform::Window*               _currentWindow = nullptr;
            math::Rect                      _cachedWindowRect;
            math::TVector2D<f32>            _cachedWindowOffest;
        };

    private:

        using WigetType = WigetWindow;
        using StateType = StateWindow;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline bool WigetWindow::isFocused() const
    {
        return Wiget::cast_data<StateType>(m_data)._isFocused;
    }

    inline const math::Dimension2D& WigetWindow::getSize() const
    {
        return Wiget::cast_data<StateType>(m_data)._size;
    }

    inline const math::Point2D& WigetWindow::getPosition() const
    {
        return  Wiget::cast_data<StateType>(m_data)._position;
    }

    inline const std::string& WigetWindow::getTitle() const
    {
        return  Wiget::cast_data<StateType>(m_data)._title;
    }

    inline const platform::Window* WigetWindow::getWindow() const
    {
        return  Wiget::cast_data<StateType>(m_data)._currentWindow;
    }

    inline WigetWindow& WigetWindow::setSize(const math::Dimension2D& size)
    {
        Wiget::cast_data<StateType>(m_data)._size = size;
        return *this;
    }

    inline WigetWindow& WigetWindow::setPosition(const math::Point2D& position)
    {
        Wiget::cast_data<StateType>(m_data)._position = position;
        return *this;
    }

    inline WigetWindow& WigetWindow::setTitle(const std::string& title)
    {
        Wiget::cast_data<StateType>(m_data)._title = title;
        return *this;
    }

    inline WigetWindow& WigetWindow::setOnSizeChanged(const OnWigetEventDimention2DParam& event)
    {
        cast_data<StateType>(m_data)._onSizeChanged = event;
        return *this;
    }

    inline WigetWindow& WigetWindow::setOnPositionChanged(const OnWigetEventPoint2DParam& event)
    {
        cast_data<StateType>(m_data)._onPositionChanged = event;
        return *this;
    }

    inline WigetWindow& WigetWindow::setOnFocusChanged(const OnWigetEventBoolParam& event)
    {
        cast_data<StateType>(m_data)._onFocusChanged = event;
        return *this;
    }

    inline WigetWindow& WigetWindow::setOnClosed(const OnWigetEvent& event)
    {
        cast_data<StateType>(m_data)._onClosed = event;
        return *this;
    }

    template<class TWiget>
    inline WigetWindow& WigetWindow::addWiget(const TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget(wiget);
        return *this;
    }

    template<class TWiget>
    inline WigetWindow& WigetWindow::addWiget(TWiget&& wiget)
    {
        static_assert(std::is_move_constructible<TWiget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWiget>::value, "should be noexcept movable");

        WigetLayout& layout = Wiget::cast_data<StateType>(m_data)._layout;
        layout.addWiget(std::forward<TWiget>(wiget));
        return *this;
    }

    inline WigetWindow& WigetWindow::setupWindowLayout(const WigetWindowLayout& layout)
    {
        cast_data<StateType>(m_data)._windowLayout = layout;
        return *this;
    }

    inline WigetWindow& WigetWindow::setupWindowLayout(WigetWindowLayout&& layout)
    {
        cast_data<StateType>(m_data)._windowLayout = std::forward<WigetWindowLayout>(layout);
        return *this;
    }

    inline TypePtr WigetWindow::getType() const
    {
        return type_of<WigetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WigetWindow>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d