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
    * @brief WidgetInputSlider class
    */
    class WidgetInputSlider final : public WidgetBase<WidgetInputSlider>
    {
    public:

        explicit WidgetInputSlider() noexcept;
        WidgetInputSlider(const WidgetInputSlider&) noexcept;
        ~WidgetInputSlider();

        TypePtr getType() const final;

        struct StateInputSlider : StateBase
        {
        };

    private:

        using WidgetType = WidgetInputSlider;
        using StateType = StateInputSlider;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline TypePtr WidgetInputSlider::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetInputSlider>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d