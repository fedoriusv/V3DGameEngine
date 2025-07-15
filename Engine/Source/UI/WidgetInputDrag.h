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
    * @brief WidgetInputDragBase interface
    */
    template<class TWidget, typename FieldType, u32 SIZE>
    class WidgetInputDragBase : public WidgetBase<TWidget>
    {
    protected:

        explicit WidgetInputDragBase(Widget::State* state) noexcept;
        WidgetInputDragBase(const WidgetInputDragBase&) noexcept;
        WidgetInputDragBase(WidgetInputDragBase&&) noexcept;
        ~WidgetInputDragBase() = default;

    public:

        TWidget& setTextColor(const color::ColorRGBAF& color);
        TWidget& setBorderColor(const color::ColorRGBAF& color);
        TWidget& setBackgroundColor(const color::ColorRGBAF& color);

        TWidget& setSize(const math::Dimension2D& size);

        template<typename... Args> requires (sizeof...(Args) == SIZE) && (std::convertible_to<Args, FieldType> && ...)
        TWidget& setValue(Args&&... args);

        struct StateInputDragBase : WidgetBase<TWidget>::StateBase
        {
            std::array<FieldType, SIZE> _value = {};
            color::ColorRGBAF           _textColor;
            color::ColorRGBAF           _borderColor;
            color::ColorRGBAF           _backgroundColor;
            math::Dimension2D           _size;
        };

    private:

        using WidgetType = WidgetInputDragBase;
        using StateType = StateInputDragBase;
    };

    template<class TWidget, typename FieldType, u32 SIZE>
    inline WidgetInputDragBase<TWidget, FieldType, SIZE>::WidgetInputDragBase(Widget::State* state) noexcept
        : WidgetBase<TWidget>(state)
    {
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    inline WidgetInputDragBase<TWidget, FieldType, SIZE>::WidgetInputDragBase(const WidgetInputDragBase& other) noexcept
        : WidgetBase<TWidget>(other)
    {
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    inline WidgetInputDragBase<TWidget, FieldType, SIZE>::WidgetInputDragBase(WidgetInputDragBase&& other) noexcept
        : WidgetBase<TWidget>(other)
    {
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    inline TWidget& WidgetInputDragBase<TWidget, FieldType, SIZE>::setTextColor(const color::ColorRGBAF& color)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._textColor = color;
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._stateMask |= Widget::State::StateMask::Color;
        return *static_cast<TWidget*>(this);
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    inline TWidget& WidgetInputDragBase<TWidget, FieldType, SIZE>::setBorderColor(const color::ColorRGBAF& color)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._borderColor = color;
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._stateMask |= Widget::State::StateMask::BorderColor;
        return *static_cast<TWidget*>(this);
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    inline TWidget& WidgetInputDragBase<TWidget, FieldType, SIZE>::setBackgroundColor(const color::ColorRGBAF& color)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._backgroundColor = color;
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._stateMask |= Widget::State::StateMask::BackgroundColor;
        return *static_cast<TWidget*>(this);
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    inline TWidget& WidgetInputDragBase<TWidget, FieldType, SIZE>::setSize(const math::Dimension2D& size)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._size = size;
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._stateMask |= Widget::State::StateMask::DefindedSize;
        return *static_cast<TWidget*>(this);
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    template<typename ...Args> requires (sizeof...(Args) == SIZE) && (std::convertible_to<Args, FieldType> && ...)
    inline TWidget& WidgetInputDragBase<TWidget, FieldType, SIZE>::setValue(Args&& ...args)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._value = { std::forward<Args>(args)... };
        return *static_cast<TWidget*>(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetInputDragInt class
    */
    class WidgetInputDragInt final : public WidgetInputDragBase<WidgetInputDragInt, s32, 1>
    {
    public:

        explicit WidgetInputDragInt(s32 value) noexcept;
        WidgetInputDragInt(const WidgetInputDragInt&) noexcept;
        WidgetInputDragInt(WidgetInputDragInt&&) noexcept;
        ~WidgetInputDragInt();

        s32 getValue() const;

        WidgetInputDragInt& setOnChangedValueEvent(const OnWidgetEventIntParam& event);

        TypePtr getType() const final;

        struct StateInputDragInt : StateInputDragBase
        {
            OnWidgetEventIntParam _onChangedValueEvent;
        };

    private:

        using WidgetType = WidgetInputDragInt;
        using StateType = StateInputDragInt;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;

    };

    inline s32 WidgetInputDragInt::getValue() const
    {
        return Widget::cast_data<StateType>(m_data)._value[0];
    }

    inline WidgetInputDragInt& WidgetInputDragInt::setOnChangedValueEvent(const OnWidgetEventIntParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    inline TypePtr WidgetInputDragInt::getType() const
    {
        return typeOf<WidgetInputDragInt>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetInputDragFloat class
    */
    class WidgetInputDragFloat final : public WidgetInputDragBase<WidgetInputDragFloat, f32, 1>
    {
    public:

        explicit WidgetInputDragFloat(f32 value) noexcept;
        WidgetInputDragFloat(const WidgetInputDragFloat&) noexcept;
        WidgetInputDragFloat(WidgetInputDragFloat&&) noexcept;
        ~WidgetInputDragFloat();

        f32 getValue() const;

        WidgetInputDragFloat& setOnChangedValueEvent(const OnWidgetEventFloatParam& event);

        TypePtr getType() const final;

        struct StateInputDragFloat : StateInputDragBase
        {
            OnWidgetEventFloatParam _onChangedValueEvent;
        };

    private:

        using WidgetType = WidgetInputDragFloat;
        using StateType = StateInputDragFloat;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;

    };

    inline f32 WidgetInputDragFloat::getValue() const
    {
        return Widget::cast_data<StateType>(m_data)._value[0];
    }

    inline WidgetInputDragFloat& WidgetInputDragFloat::setOnChangedValueEvent(const OnWidgetEventFloatParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    inline TypePtr WidgetInputDragFloat::getType() const
    {
        return typeOf<WidgetInputDragFloat>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetInputDragInt>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WidgetInputDragFloat>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d