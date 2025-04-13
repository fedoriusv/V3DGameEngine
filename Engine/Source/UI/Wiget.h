#pragma once

#include "Common.h"
#include "WigetLayout.h"

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

    /**
    * @brief Wiget base class
    */
    class Wiget
    {
    public:

        struct Context
        {
            Context() noexcept = default;
            ~Context() = default;
        };

        explicit Wiget(Context* context) noexcept;
        Wiget(const Wiget& other) noexcept;
        virtual ~Wiget() = default;

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

        bool isStateMaskActive(u64 mask) const;

        struct ContextBase : Context
        {
            std::string             _toolTip;
            OnWigetEvent            _onVisibleChanged;
            OnWigetEvent            _onActiveChanged;
            OnWigetEventUpdate      _onUpdate;
            bool                    _isActive         = true;
            bool                    _isVisible        = true;
            bool                    _isPressed        = false;
            bool                    _isHovered        = false;
            bool                    _showToolTip      = false;
            bool                    _unused[3]        = {};

            //0x01 - Active
            //0x02 - Visible
            //0x04 - ForceUpdate
            //...
            //0x80 - Reserved
            u64                     _stateMask        = 0;
        };

    protected:

        friend WigetLayout;
        friend WigetHandler;

        template<class TContext = ContextBase>
        static TContext& cast_data(Context* context)
        {
            return *static_cast<TContext*>(context);
        }

        virtual bool update(WigetHandler* handler, Wiget* parent, WigetLayout* layout, f32 dt);

        Context* m_data;
    };

    inline bool Wiget::isActive() const
    {
        return cast_data<ContextBase>(m_data)._isActive;
    }

    inline bool Wiget::isVisible() const
    {
        return cast_data<ContextBase>(m_data)._isVisible;
    }

    inline bool Wiget::isShowToolTip() const
    {
        return cast_data<ContextBase>(m_data)._showToolTip;
    }

    inline const std::string& Wiget::getToolTip() const
    {
        return cast_data<ContextBase>(m_data)._toolTip;
    }

    inline Wiget& Wiget::setActive(bool active)
    {
        if (cast_data<ContextBase>(m_data)._isActive != active)
        {
            cast_data<ContextBase>(m_data)._isActive = active;
            cast_data<ContextBase>(m_data)._stateMask = 0x01; //Active
        }

        return *this;
    }

    inline Wiget& Wiget::setVisible(bool visible)
    {
        if (cast_data<ContextBase>(m_data)._isVisible != visible)
        {
            cast_data<ContextBase>(m_data)._isVisible = visible;
            cast_data<ContextBase>(m_data)._stateMask = 0x02; //Visible
        }

        return *this;
    }

    inline Wiget& Wiget::setToolTip(bool show, const std::string& tip)
    {
        cast_data<ContextBase>(m_data)._showToolTip = show;
        cast_data<ContextBase>(m_data)._toolTip = tip;
        return *this;
    }

    inline Wiget& Wiget::setOnUpdate(const OnWigetEventUpdate& event)
    {
        cast_data<ContextBase>(m_data)._onUpdate = event;
        return *this;
    }

    inline Wiget& Wiget::setOnVisibleChanged(const OnWigetEvent& event)
    {
        cast_data<ContextBase>(m_data)._onVisibleChanged = event;
        return *this;
    }

    inline Wiget& Wiget::setOnActiveChanged(const OnWigetEvent& event)
    {
        cast_data<ContextBase>(m_data)._onActiveChanged = event;
        return *this;
    }

    inline bool Wiget::isStateMaskActive(u64 mask) const
    {
        return cast_data<ContextBase>(m_data)._stateMask & mask;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetBase template wiget class
    */
    template<class TWiget>
    class WigetBase : public Wiget
    {
    public:

        explicit WigetBase(Wiget::Context* context) noexcept;
        WigetBase(const WigetBase&) noexcept;
        ~WigetBase() = default;

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
    inline WigetBase<TWiget>::WigetBase(Wiget::Context* context) noexcept
        : Wiget(context)
    {
    }

    template<class TWiget>
    inline WigetBase<TWiget>::WigetBase(const WigetBase& other) noexcept
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