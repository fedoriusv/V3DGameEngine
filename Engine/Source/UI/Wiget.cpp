#include "Wiget.h"
#include "WigetHandler.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace ui
{

 Wiget::Wiget() noexcept
     : m_toolTip("")
     , m_isActive(true)
     , m_isVisible(true)
     , m_showToolTip(false)
 {
 }

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetButton::WigetButton(const std::string& title) noexcept
    : Wiget()
    , m_title(title)
{
}

bool WigetButton::update(WigetHandler* handler, f32 dt)
{
    ASSERT(handler, "must be valid");
    return handler->drawButton(this, dt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WigetMenu::MenuItem::MenuItem(const std::string& title) noexcept
    : m_title(title)
{
}

bool WigetMenu::MenuItem::update(WigetHandler* handler, f32 dt)
{
    return handler->drawMenuItem(this, dt);
}

WigetMenu::WigetMenu(const std::string& title) noexcept
    : Wiget()
    , m_title(title)
{
}

WigetMenu::~WigetMenu()
{
    for (auto& item : m_wigets)
    {
        //delete item;
    }
}

const std::string& WigetMenu::MenuItem::getTitle() const
{
    return m_title;
}

WigetMenu::MenuItem* WigetMenu::MenuItem::setTitle(const std::string& title)
{
    m_title = title;
    return this;
}

WigetMenu::MenuItem* WigetMenu::MenuItem::setOnClickedEvent(const OnWigetEvent& event)
{
    m_onClickedEvent = event;
    return this;
}

bool WigetMenu::update(WigetHandler* handler, f32 dt)
{
    ASSERT(handler, "must be valid");
    if (handler->beginDrawMenu(this, dt))
    {
        for (auto& item : m_wigets)
        {
            item->update(handler, dt);
        }

        return handler->endDrawMenu(this, dt);
    }

    return true;
}

} // namespace ui
} // namespace v3d