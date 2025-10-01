#pragma once

#include "Common.h"
#include "Utils/Copiable.h"
#include "Utils/Observable.h"
#include "Scene/Transform.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Widget;
    class WidgetHandler;
    class WidgetLayout;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using OnWidgetEvent = std::function<void(Widget*)>;
    using OnWidgetEventBoolParam = std::function<void(Widget*, bool)>;
    using OnWidgetEventIntParam = std::function<void(Widget*, s32)>;
    using OnWidgetEventInt2Param = std::function<void(Widget*, const math::int2&)>;
    using OnWidgetEventInt3Param = std::function<void(Widget*, const math::int3&)>;
    using OnWidgetEventFloatParam = std::function<void(Widget*, f32)>;
    using OnWidgetEventFloat2Param = std::function<void(Widget*, const math::float2&)>;
    using OnWidgetEventFloat3Param = std::function<void(Widget*, const math::float3&)>;
    using OnWidgetEventColorParam = std::function<void(Widget*, const color::ColorRGBAF&)>;
    using OnWidgetEventStringParam = std::function<void(Widget*, const std::string&)>;
    using OnWidgetEventVoidParam = std::function<void(Widget*, void*)>;

    using OnWidgetEventDimention2DParam = std::function<void(Widget*, Widget*, const math::Dimension2D&)>;
    using OnWidgetEventPoint2DParam = std::function<void(Widget*, Widget*, const math::Point2D&)>;
    using OnWidgetEventRectParam = std::function<void(Widget*, Widget*, const math::Rect&)>;
    using OnWidgetEventMatrix4x4Param = std::function<void(Widget*, Widget*, const math::Matrix4D&)>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct WidgetReport
    {
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Widget base class
    */
    class Widget : public utils::Observer<WidgetReport>, protected utils::Copiable<Widget>
    {
    public:

        struct State
        {
            enum StateMask
            {
                Active = 0x1,
                Visible = 0x2,
                Selected = 0x4,
                MainLayout = 0x8,
                HorizontalLine = 0x10,
                DefindedSize = 0x20,
                Color = 0x40,
                HoveredColor = 0x80,
                ClickedColor = 0x100,
                BorderColor = 0x200,
                BackgroundColor = 0x400,

                FirstUpdateState = 0x800,
                ForceUpdateState = 0x1000,
                ChildLayoutState = 0x2000,
                CollapsedState = 0x4000,
            };

            State() noexcept = default;
            ~State() = default;

            u64         _uid       = ~0;
            u64         _stateMask = 0;
            math::Rect  _itemRect;
        };

        virtual ~Widget();

        bool isActive() const;
        bool isVisible() const;
        bool isShowToolTip() const;

        u64 getID() const;
        const std::string& getToolTip() const;

        Widget& setActive(bool active);
        Widget& setVisible(bool visible);
        Widget& setToolTip(bool show, const std::string& tip = "");

        Widget& setOnCreated(const OnWidgetEvent& event);
        Widget& setOnUpdate(const OnWidgetEventFloatParam& event);
        Widget& setOnVisibleChanged(const OnWidgetEvent& event);
        Widget& setOnActiveChanged(const OnWidgetEvent& event);

        bool isStateMaskActive(u64 state) const;

        struct StateBase : State
        {
            std::string             _toolTip;
            OnWidgetEvent           _onCreated;
            OnWidgetEventFloatParam _onUpdate;
            OnWidgetEvent           _onVisibleChanged;
            OnWidgetEvent           _onActiveChanged;
            bool                    _isActive         = true;
            bool                    _isVisible        = true;
            bool                    _isPressed        = false;
            bool                    _isHovered        = false;
            bool                    _isCreated        = false;
            bool                    _isDestroyed      = false;
            bool                    _isFocused        = false;
            bool                    _showToolTip      = false;
        };

        virtual TypePtr getType() const = 0;

        virtual bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt);
        virtual math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout);

        void handleNotify(const utils::Reporter<WidgetReport>* reporter, const WidgetReport& data) override;

    protected:

        explicit Widget(State* state) noexcept;
        Widget(const Widget& other) noexcept;
        Widget(Widget&& other) noexcept;

        template<class TState = StateBase>
        static TState& cast_data(State* state)
        {
            return *static_cast<TState*>(state);
        }

        State* m_data = nullptr;

        friend WidgetHandler;
    };

    inline bool Widget::isActive() const
    {
        return cast_data<StateBase>(m_data)._isActive;
    }

    inline bool Widget::isVisible() const
    {
        return cast_data<StateBase>(m_data)._isVisible;
    }

    inline bool Widget::isShowToolTip() const
    {
        return cast_data<StateBase>(m_data)._showToolTip;
    }

    inline u64 Widget::getID() const
    {
        return cast_data<StateBase>(m_data)._uid;
    }

    inline const std::string& Widget::getToolTip() const
    {
        return cast_data<StateBase>(m_data)._toolTip;
    }

    inline Widget& Widget::setActive(bool active)
    {
        if (cast_data<StateBase>(m_data)._isActive != active)
        {
            cast_data<StateBase>(m_data)._isActive = active;
            cast_data<StateBase>(m_data)._stateMask = State::StateMask::Active;
        }

        return *this;
    }

    inline Widget& Widget::setVisible(bool visible)
    {
        if (cast_data<StateBase>(m_data)._isVisible != visible)
        {
            cast_data<StateBase>(m_data)._isVisible = visible;
            cast_data<StateBase>(m_data)._stateMask = State::StateMask::Visible;
        }

        return *this;
    }

    inline Widget& Widget::setToolTip(bool show, const std::string& tip)
    {
        cast_data<StateBase>(m_data)._showToolTip = show;
        cast_data<StateBase>(m_data)._toolTip = tip;
        return *this;
    }

    inline Widget& Widget::setOnUpdate(const OnWidgetEventFloatParam& event)
    {
        cast_data<StateBase>(m_data)._onUpdate = event;
        return *this;
    }

    inline Widget& Widget::setOnVisibleChanged(const OnWidgetEvent& event)
    {
        cast_data<StateBase>(m_data)._onVisibleChanged = event;
        return *this;
    }

    inline Widget& Widget::setOnActiveChanged(const OnWidgetEvent& event)
    {
        cast_data<StateBase>(m_data)._onActiveChanged = event;
        return *this;
    }

    inline Widget& Widget::setOnCreated(const OnWidgetEvent& event)
    {
        cast_data<StateBase>(m_data)._onCreated = event;
        return *this;
    }

    inline bool Widget::isStateMaskActive(u64 state) const
    {
        return cast_data<StateBase>(m_data)._stateMask & state;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetBase template Widget class
    */
    template<class TWidget>
    class WidgetBase : public Widget
    {
    public:

        bool isActive() const;
        bool isVisible() const;
        bool isShowToolTip() const;

        const std::string& getToolTip() const;

        TWidget& setActive(bool active);
        TWidget& setVisible(bool visible);
        TWidget& setToolTip(bool show, const std::string& tip);

        TWidget& setOnCreated(const OnWidgetEvent& event);
        TWidget& setOnUpdate(const OnWidgetEventFloatParam& event);
        TWidget& setOnVisibleChanged(const OnWidgetEvent& event);
        TWidget& setOnActiveChanged(const OnWidgetEvent& event);

    protected:

        explicit WidgetBase(Widget::State* state) noexcept;
        WidgetBase(const WidgetBase&) noexcept;
        WidgetBase(WidgetBase&&) noexcept;
        virtual ~WidgetBase() = default;
    };

    template<class TWidget>
    inline WidgetBase<TWidget>::WidgetBase(Widget::State* context) noexcept
        : Widget(context)
    {
    }

    template<class TWidget>
    inline WidgetBase<TWidget>::WidgetBase(const WidgetBase& other) noexcept
        : Widget(other)
    {
    }

    template<class TWidget>
    inline WidgetBase<TWidget>::WidgetBase(WidgetBase&& other) noexcept
        : Widget(other)
    {
    }

    template<class TWidget>
    inline bool WidgetBase<TWidget>::isActive() const
    {
        return Widget::isActive();
    }

    template<class TWidget>
    inline bool WidgetBase<TWidget>::isVisible() const
    {
        return Widget::isVisible();
    }

    template<class TWidget>
    inline bool WidgetBase<TWidget>::isShowToolTip() const
    {
        return Widget::isShowToolTip();
    }

    template<class TWidget>
    inline const std::string& WidgetBase<TWidget>::getToolTip() const
    {
        return Widget::getToolTip();
    }

    template<class TWidget>
    inline TWidget& WidgetBase<TWidget>::setActive(bool active)
    {
        return *static_cast<TWidget*>(&Widget::setActive(active));
    }

    template<class TWidget>
    inline TWidget& WidgetBase<TWidget>::setVisible(bool visible)
    {
        return *static_cast<TWidget*>(&Widget::setVisible(visible));
    }

    template<class TWidget>
    inline TWidget& WidgetBase<TWidget>::setOnCreated(const OnWidgetEvent& event)
    {
        return *static_cast<TWidget*>(&Widget::setOnCreated(event));
    }

    template<class TWidget>
    inline TWidget& WidgetBase<TWidget>::setToolTip(bool show, const std::string& tip)
    {
        return *static_cast<TWidget*>(&Widget::setToolTip(show, tip));
    }

    template<class TWidget>
    inline TWidget& WidgetBase<TWidget>::setOnUpdate(const OnWidgetEventFloatParam& event)
    {
        return *static_cast<TWidget*>(&Widget::setOnUpdate(event));
    }

    template<class TWidget>
    inline TWidget& WidgetBase<TWidget>::setOnVisibleChanged(const OnWidgetEvent& event)
    {
        return *static_cast<TWidget*>(&Widget::setOnVisibleChanged(event));
    }

    template<class TWidget>
    inline TWidget& WidgetBase<TWidget>::setOnActiveChanged(const OnWidgetEvent& event)
    {
        return *static_cast<TWidget*>(&Widget::setOnActiveChanged(event));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

/**
    * @brief WidgetTEST class. Test wiget. Do here what you whant
    */
    class WidgetTEST final : public WidgetBase<WidgetTEST>
    {
    public:

        explicit WidgetTEST() noexcept;
        WidgetTEST(const WidgetTEST&) noexcept;
        WidgetTEST(WidgetTEST&&) noexcept;
        ~WidgetTEST();

        WidgetTEST& setParam(const std::string& value);
        WidgetTEST& setParam(const color::ColorRGBAF& value);
        WidgetTEST& setParam(f32 value);

        TypePtr getType() const final;

        struct StateTEST : StateBase
        {
            std::string         _stringParam;
            color::ColorRGBAF   _colorParam;
            f32                 _valueParam;
        };

    private:

        using WidgetType = WidgetTEST;
        using StateType = StateTEST;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline WidgetTEST& WidgetTEST::setParam(const std::string& value)
    {
        Widget::cast_data<StateType>(m_data)._stringParam = value;
        return *this;
    }

    inline WidgetTEST& WidgetTEST::setParam(const color::ColorRGBAF& value)
    {
        Widget::cast_data<StateType>(m_data)._colorParam = value;
        Widget::cast_data<StateType>(m_data)._stateMask |= Widget::State::StateMask::Color;
        return *this;
    }

    inline WidgetTEST& WidgetTEST::setParam(f32 value)
    {
        Widget::cast_data<StateType>(m_data)._valueParam = value;
        return *this;
    }

    inline TypePtr WidgetTEST::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetTEST>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d