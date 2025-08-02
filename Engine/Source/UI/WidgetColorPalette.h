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
        WidgetColorPalette(WidgetColorPalette&&) noexcept;
        ~WidgetColorPalette();

        WidgetColorPalette& setColor(const color::ColorRGBAF& color);
        const color::ColorRGBAF getColor() const;

        WidgetColorPalette& setOnColorChangedEvent(const OnWidgetEventColorParam& event);

        TypePtr getType() const final;

        struct StateColorPalette : StateBase
        {
            color::ColorRGBAF       _color = { 1.f, 1.f, 1.f, 1.f };
            OnWidgetEventColorParam _onColorChanged;
        };

    private:

        using WidgetType = WidgetColorPalette;
        using StateType = StateColorPalette;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;
    };

    inline WidgetColorPalette& WidgetColorPalette::setColor(const color::ColorRGBAF& color)
    {
        Widget::cast_data<StateType>(m_data)._color = color;
        return *this;
    }

    inline const color::ColorRGBAF WidgetColorPalette::getColor() const
    {
        return Widget::cast_data<StateType>(m_data)._color;
    }

    inline WidgetColorPalette& WidgetColorPalette::setOnColorChangedEvent(const OnWidgetEventColorParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onColorChanged = event;
        return *this;
    }

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