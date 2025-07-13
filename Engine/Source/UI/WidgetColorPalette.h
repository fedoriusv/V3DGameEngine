#pragma once

#include "Common.h"
#include "Widget.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetColorPalette class
    */
    class WidgetColorPalette final : public WidgetBase<WidgetColorPalette>
    {
    public:

        explicit WidgetColorPalette() noexcept;
        WidgetColorPalette(const WidgetColorPalette&) noexcept;
        ~WidgetColorPalette();

        TypePtr getType() const final;

        struct StateColorPalette : StateBase
        {
        };

    private:

        using WidgetType = WidgetColorPalette;
        using StateType = StateColorPalette;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline TypePtr WidgetColorPalette::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetColorPalette>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d