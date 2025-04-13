#pragma once

#include "Wiget.h"

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

        template<class UWiget>
        WigetTabBar& addWiget(const UWiget& wiget);

        struct ContextTabBar : ContextBase
        {
            WigetLayout _layout;
        };

    private:

        bool update(WigetHandler* handler, Wiget* parent, WigetLayout* layout, f32 dt) override;
    };

    template<class TWiget>
    inline WigetTabBar& WigetTabBar::addWiget(const TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<ContextTabBar>(m_data)._layout;
        layout.addWiget<TWiget>(wiget);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d