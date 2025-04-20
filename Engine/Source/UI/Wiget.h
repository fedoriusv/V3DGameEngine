#pragma once

#include "Common.h"
#include "Utils/Copiable.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Wiget;
    class WigetHandler;
    class WigetLayout;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using OnWigetEvent = std::function<void(const Wiget*)>;
    using OnWigetEventBoolParam = std::function<void(const Wiget*, bool)>;
    using OnWigetEventIntParam = std::function<void(const Wiget*, s32)>;
    using OnWigetEventFloatParam = std::function<void(const Wiget*, f32)>;


    using OnWigetEventUpdate = std::function<void(Wiget*, f32)>;

    using OnWigetEventDimention2DParam = std::function<void(const Wiget*, const Wiget*, const math::Dimension2D&)>;
    using OnWigetEventPoint2DParam = std::function<void(const Wiget*, const Wiget*, const math::Point2D&)>;
    using OnWigetEventRect32Param = std::function<void(const Wiget*, const Wiget*, const math::Rect32&)>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct WigetReport
    {
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Wiget base class
    */
    class Wiget : public utils::Observer<WigetReport>, protected utils::Copiable<Wiget>
    {
    public:

        struct State
        {
            enum StateMask
            {
                Active = 0x01,
                Visible = 0x02,
                ForceUpdate = 0x04,
                HorizontalLine = 0x08,
                WindowLayout = 0x10, //?
                MenuLayout = 0x20, //?
                FirstUpdate = 0x40,
                ChildLayout = 0x80,

                Color = 0x100,
                HoveredColor = 0x200,
                ClickedColor = 0x400
            };

            State() noexcept = default;
            ~State() = default;

            u64             _uid       = ~0;
            u64             _stateMask = 0;
            math::RectF32  _itemRect;
        };

        virtual ~Wiget();

        bool isActive() const;
        bool isVisible() const;
        bool isShowToolTip() const;

        const std::string& getToolTip() const;

        Wiget& setActive(bool active);
        Wiget& setVisible(bool visible);
        Wiget& setToolTip(bool show, const std::string& tip = "");

        Wiget& setOnUpdate(const OnWigetEventUpdate& event);
        Wiget& setOnVisibleChanged(const OnWigetEvent& event);
        Wiget& setOnActiveChanged(const OnWigetEvent& event);
        Wiget& setOnCreated(const OnWigetEvent& event);

        bool isStateMaskActive(u64 state) const;

        struct StateBase : State
        {
            std::string             _toolTip;
            OnWigetEvent            _onCreated;
            OnWigetEvent            _onVisibleChanged;
            OnWigetEvent            _onActiveChanged;
            OnWigetEventUpdate      _onUpdate;
            bool                    _isActive         = true;
            bool                    _isVisible        = true;
            bool                    _isPressed        = false;
            bool                    _isHovered        = false;
            bool                    _showToolTip      = false;
            bool                    _isCreated        = false;
            bool                    _isDestroyed      = false;
            bool                    _unused[1]        = {};
        };

        virtual bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt);
        virtual math::Vector2D calculateSize(WigetHandler* handler, Wiget* parent, Wiget* layout);

        void handleNotify(const utils::Reporter<WigetReport>* reporter, const WigetReport& data) override;

    protected:

        explicit Wiget(State* state) noexcept;
        Wiget(const Wiget& other) noexcept;
        Wiget(Wiget&& other) noexcept;

        template<class TState = StateBase>
        static TState& cast_data(State* state)
        {
            return *static_cast<TState*>(state);
        }

        State* m_data = nullptr;

        friend WigetHandler;
    };

    inline bool Wiget::isActive() const
    {
        return cast_data<StateBase>(m_data)._isActive;
    }

    inline bool Wiget::isVisible() const
    {
        return cast_data<StateBase>(m_data)._isVisible;
    }

    inline bool Wiget::isShowToolTip() const
    {
        return cast_data<StateBase>(m_data)._showToolTip;
    }

    inline const std::string& Wiget::getToolTip() const
    {
        return cast_data<StateBase>(m_data)._toolTip;
    }

    inline Wiget& Wiget::setActive(bool active)
    {
        if (cast_data<StateBase>(m_data)._isActive != active)
        {
            cast_data<StateBase>(m_data)._isActive = active;
            cast_data<StateBase>(m_data)._stateMask = State::StateMask::Active;
        }

        return *this;
    }

    inline Wiget& Wiget::setVisible(bool visible)
    {
        if (cast_data<StateBase>(m_data)._isVisible != visible)
        {
            cast_data<StateBase>(m_data)._isVisible = visible;
            cast_data<StateBase>(m_data)._stateMask = State::StateMask::Visible;
        }

        return *this;
    }

    inline Wiget& Wiget::setToolTip(bool show, const std::string& tip)
    {
        cast_data<StateBase>(m_data)._showToolTip = show;
        cast_data<StateBase>(m_data)._toolTip = tip;
        return *this;
    }

    inline Wiget& Wiget::setOnUpdate(const OnWigetEventUpdate& event)
    {
        cast_data<StateBase>(m_data)._onUpdate = event;
        return *this;
    }

    inline Wiget& Wiget::setOnVisibleChanged(const OnWigetEvent& event)
    {
        cast_data<StateBase>(m_data)._onVisibleChanged = event;
        return *this;
    }

    inline Wiget& Wiget::setOnActiveChanged(const OnWigetEvent& event)
    {
        cast_data<StateBase>(m_data)._onActiveChanged = event;
        return *this;
    }

    inline Wiget& Wiget::setOnCreated(const OnWigetEvent& event)
    {
        cast_data<StateBase>(m_data)._onCreated = event;
        return *this;
    }

    inline bool Wiget::isStateMaskActive(u64 state) const
    {
        return cast_data<StateBase>(m_data)._stateMask & state;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetBase template wiget class
    */
    template<class TWiget>
    class WigetBase : public Wiget
    {
    public:

        explicit WigetBase(Wiget::State* state) noexcept;
        WigetBase(const WigetBase&) noexcept;
        WigetBase(WigetBase&&) noexcept;
        virtual ~WigetBase() = default;

        bool isActive() const;
        bool isVisible() const;
        bool isShowToolTip() const;

        const std::string& getToolTip() const;

        TWiget& setActive(bool active);
        TWiget& setVisible(bool visible);
        TWiget& setToolTip(bool show, const std::string& tip);

        TWiget& setOnUpdate(const OnWigetEventUpdate& event);
        TWiget& setOnVisibleChanged(const OnWigetEvent& event);
        TWiget& setOnActiveChanged(const OnWigetEvent& event);
    };

    template<class TWiget>
    inline WigetBase<TWiget>::WigetBase(Wiget::State* context) noexcept
        : Wiget(context)
    {
    }

    template<class TWiget>
    inline WigetBase<TWiget>::WigetBase(const WigetBase& other) noexcept
        : Wiget(other)
    {
    }

    template<class TWiget>
    inline WigetBase<TWiget>::WigetBase(WigetBase&& other) noexcept
        : Wiget(other)
    {
    }

    template<class TWiget>
    inline bool WigetBase<TWiget>::isActive() const
    {
        return Wiget::isActive();
    }

    template<class TWiget>
    inline bool WigetBase<TWiget>::isVisible() const
    {
        return Wiget::isVisible();
    }

    template<class TWiget>
    inline bool WigetBase<TWiget>::isShowToolTip() const
    {
        return Wiget::isShowToolTip();
    }

    template<class TWiget>
    inline const std::string& WigetBase<TWiget>::getToolTip() const
    {
        return Wiget::getToolTip();
    }

    template<class TWiget>
    inline TWiget& WigetBase<TWiget>::setActive(bool active)
    {
        return *static_cast<TWiget*>(&Wiget::setActive(active));
    }

    template<class TWiget>
    inline TWiget& WigetBase<TWiget>::setVisible(bool visible)
    {
        return *static_cast<TWiget*>(&Wiget::setVisible(visible));
    }

    template<class TWiget>
    inline TWiget& WigetBase<TWiget>::setToolTip(bool show, const std::string& tip)
    {
        return *static_cast<TWiget*>(&Wiget::setToolTip(show, tip));
    }

    template<class TWiget>
    inline TWiget& WigetBase<TWiget>::setOnUpdate(const OnWigetEventUpdate& event)
    {
        return *static_cast<TWiget*>(&Wiget::setOnUpdate(event));
    }

    template<class TWiget>
    inline TWiget& WigetBase<TWiget>::setOnVisibleChanged(const OnWigetEvent& event)
    {
        return *static_cast<TWiget*>(&Wiget::setOnVisibleChanged(event));
    }

    template<class TWiget>
    inline TWiget& WigetBase<TWiget>::setOnActiveChanged(const OnWigetEvent& event)
    {
        return *static_cast<TWiget*>(&Wiget::setOnActiveChanged(event));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d