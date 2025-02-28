#pragma once

#include "Common.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Wiget;
    class WigetMenu;
    class WigetLayout;
    class WigetHandler;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using OnWigetEvent = std::function<void (const Wiget*)>;

    class Wiget
    {
    public:

        explicit Wiget() noexcept;
        virtual ~Wiget() = default;

        bool isActive() const;
        bool isVisible() const;
        bool isShowToolTip() const;

        const std::string& getToolTip() const;

        template<class TWiget = Wiget>
        TWiget* setActive(bool active);

        template<class TWiget = Wiget>
        TWiget* setVisible(bool visible);

        template<class TWiget = Wiget>
        TWiget* setToolTip(bool show, const std::string& tip = "");

        template<class TWiget = Wiget>
        TWiget* setOnVisibleChanged(const OnWigetEvent& event);

        template<class TWiget = Wiget>
        TWiget* setOnActiveChanged(const OnWigetEvent& event);

    protected:

        friend WigetLayout;
        friend WigetMenu;

        virtual bool update(WigetHandler* handler, f32 dt) = 0;

    public:

        std::string     m_toolTip;
        OnWigetEvent    m_onVisibleChanged;
        OnWigetEvent    m_onActiveChanged;
        bool            m_isActive;
        bool            m_isVisible;
        bool            m_showToolTip;

    };

    inline bool Wiget::isActive() const
    {
        return m_isActive;
    }

    inline bool Wiget::isVisible() const
    {
        return m_isVisible;
    }

    inline bool Wiget::isShowToolTip() const
    {
        return m_showToolTip;
    }

    inline const std::string& Wiget::getToolTip() const
    {
        return m_toolTip;
    }

    template<class TWiget>
    inline TWiget* Wiget::setActive(bool active)
    {
        if (m_isActive != active)
        {
            m_isActive = active;
            if (m_onActiveChanged)
            {
                m_onActiveChanged(this);
            }
        }

        return static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget* Wiget::setVisible(bool visible)
    {
        if (m_isVisible != visible)
        {
            m_isVisible = visible;
            if (m_onVisibleChanged)
            {
                m_onVisibleChanged(this);
            }
        }

        return static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget* Wiget::setToolTip(bool show, const std::string& tip)
    {
        m_showToolTip = show;
        m_toolTip = tip;
        return static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget* Wiget::setOnVisibleChanged(const OnWigetEvent& event)
    {
        m_onVisibleChanged = event;
        return static_cast<TWiget*>(this);

    }

    template<class TWiget>
    inline TWiget* Wiget::setOnActiveChanged(const OnWigetEvent& event)
    {
        m_onActiveChanged = event;
        return static_cast<TWiget*>(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetButton : public Wiget
    {
    public:

        explicit WigetButton(const std::string& title) noexcept;
        ~WigetButton() = default;

        const std::string& getTitle() const;

        template<class TWiget = WigetButton>
        TWiget* setTitle(const std::string& title);

        template<class TWiget = WigetButton>
        TWiget* setOnClickedEvent(const OnWigetEvent& event);

        template<class TWiget = WigetButton>
        TWiget* setOnHoveredEvent(const OnWigetEvent& event);

    private:

        bool update(WigetHandler* handler, f32 dt) override;

    public:

        std::string     m_title;
        OnWigetEvent    m_onClickedEvent;
        OnWigetEvent    m_onPressedEvent;
        OnWigetEvent    m_onReleasedEvent;
        OnWigetEvent    m_onHoveredEvent;
        OnWigetEvent    m_onUnhoveredEvent;
    };

    inline const std::string& WigetButton::getTitle() const
    {
        return m_title;
    }

    template<class TWiget>
    inline TWiget* WigetButton::setTitle(const std::string& title)
    {
        m_title = title;
        return static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget* WigetButton::setOnClickedEvent(const OnWigetEvent& event)
    {
        m_onClickedEvent = event;
        return static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget* WigetButton::setOnHoveredEvent(const OnWigetEvent& event)
    {
        m_onHoveredEvent = event;
        return static_cast<TWiget*>(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetMenu : public Wiget
    {
    public:

        class MenuItem final : public Wiget
        {
        public:

            explicit MenuItem(const std::string& title) noexcept;
            ~MenuItem() = default;

            const std::string& getTitle() const;

            MenuItem* setTitle(const std::string& title);
            MenuItem* setOnClickedEvent(const OnWigetEvent& event);

        private:

            bool update(WigetHandler* handler, f32 dt) override;

        public:

            std::string  m_title;
            OnWigetEvent m_onClickedEvent;
        };

        explicit WigetMenu(const std::string& title) noexcept;
        ~WigetMenu();

        const std::string& getTitle() const;

        template<class TWiget = WigetMenu>
        TWiget* addWiget(Wiget* wiget);

        template<class TWiget = WigetMenu>
        TWiget* setTitle(const std::string& title);

        template<class TWiget = WigetMenu>
        TWiget* setOnClickedEvent(const OnWigetEvent& event);

    private:

        bool update(WigetHandler* handler, f32 dt) override;

    public:

        std::string             m_title;
        std::vector<Wiget*>     m_wigets;

        OnWigetEvent            m_onClickedEvent;
    };

    inline const std::string& WigetMenu::getTitle() const
    {
        return m_title;
    }

    template<class TWiget>
    inline TWiget* WigetMenu::addWiget(Wiget* wiget)
    {
        m_wigets.push_back(wiget);
        return static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget* WigetMenu::setTitle(const std::string& title)
    {
        m_title = title;
        return static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget* WigetMenu::setOnClickedEvent(const OnWigetEvent& event)
    {
        m_onClickedEvent = event;
        return static_cast<TWiget*>(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d