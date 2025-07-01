#pragma once

#include "Common.h"
#include "Widget.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetGroupBase interface
    */
    template<class TWidget, typename FieldType, u32 SIZE>
    class WidgetInputFieldBase : public WidgetBase<TWidget>
    {
    protected:

        explicit WidgetInputFieldBase(Widget::State* state) noexcept;
        WidgetInputFieldBase(const WidgetInputFieldBase&) noexcept;
        WidgetInputFieldBase(WidgetInputFieldBase&&) noexcept;
        ~WidgetInputFieldBase() = default;

    public:

        template<typename... Args> requires (sizeof...(Args) == SIZE) && (std::convertible_to<Args, FieldType> && ...)
        void setValue(Args&&... args);

        struct StateInputFieldBase : WidgetBase<TWidget>::StateBase
        {
            std::array<FieldType, SIZE> _value = {};
        };

    private:

        using WidgetType = WidgetInputFieldBase;
        using StateType = StateInputFieldBase;
    };

    template<class TWidget, typename FieldType, u32 SIZE>
    inline WidgetInputFieldBase<TWidget, FieldType, SIZE>::WidgetInputFieldBase(Widget::State* state) noexcept
        : WidgetBase<TWidget>(state)
    {
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    inline WidgetInputFieldBase<TWidget, FieldType, SIZE>::WidgetInputFieldBase(const WidgetInputFieldBase& other) noexcept
        : WidgetBase<TWidget>(other)
    {
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    inline WidgetInputFieldBase<TWidget, FieldType, SIZE>::WidgetInputFieldBase(WidgetInputFieldBase&& other) noexcept
        : WidgetBase<TWidget>(other)
    {
    }

    template<class TWidget, typename FieldType, u32 SIZE>
    template<typename ...Args> requires (sizeof...(Args) == SIZE) && (std::convertible_to<Args, FieldType> && ...)
    inline void WidgetInputFieldBase<TWidget, FieldType, SIZE>::setValue(Args&& ...args)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._value = { std::forward<Args>(args)... };
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetInputText class
    */
    class WidgetInputText final : public WidgetInputFieldBase<WidgetInputText, std::string, 1>
    {
    public:

        explicit WidgetInputText(const std::string& text) noexcept;
        explicit WidgetInputText(const std::string& text, const math::Dimension2D& size) noexcept;
        WidgetInputText(const WidgetInputText&) noexcept;
        WidgetInputText(WidgetInputText&&) noexcept;
        ~WidgetInputText();

        const std::string& getValue() const;

        WidgetInputText& setOnChangedValueEvent(const OnWidgetEventStringParam& event);

        TypePtr getType() const final;

        struct StateInputText : StateInputFieldBase
        {
            OnWidgetEventStringParam _onChangedValueEvent;
            math::Dimension2D        _size;
            bool                     _multiline = false;
        };

    private:

        using WidgetType = WidgetInputText;
        using StateType = StateInputText;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline const std::string& WidgetInputText::getValue() const
    {
        return Widget::cast_data<StateType>(m_data)._value[0];
    }

    inline WidgetInputText& WidgetInputText::setOnChangedValueEvent(const OnWidgetEventStringParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    inline TypePtr WidgetInputText::getType() const
    {
        return typeOf<WidgetInputText>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetInputInt class
    */
    class WidgetInputInt final : public WidgetInputFieldBase<WidgetInputInt, s32, 1>
    {
    public:

        explicit WidgetInputInt(s32 value) noexcept;
        WidgetInputInt(const WidgetInputInt&) noexcept;
        WidgetInputInt(WidgetInputInt&&) noexcept;
        ~WidgetInputInt();

        s32 getValue() const;

        WidgetInputInt& setOnChangedValueEvent(const OnWidgetEventIntParam& event);

        TypePtr getType() const final;

        struct StateInputInt : StateInputFieldBase
        {
            OnWidgetEventIntParam _onChangedValueEvent;
        };

    private:

        using WidgetType = WidgetInputInt;
        using StateType = StateInputInt;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;

    };

    inline s32 WidgetInputInt::getValue() const
    {
        return Widget::cast_data<StateType>(m_data)._value[0];
    }

    inline WidgetInputInt& WidgetInputInt::setOnChangedValueEvent(const OnWidgetEventIntParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    inline TypePtr WidgetInputInt::getType() const
    {
        return typeOf<WidgetInputInt>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetInputFloat class
    */
    class WidgetInputFloat final : public WidgetInputFieldBase<WidgetInputFloat, f32, 1>
    {
    public:

        explicit WidgetInputFloat(f32 value) noexcept;
        WidgetInputFloat(const WidgetInputFloat&) noexcept;
        WidgetInputFloat(WidgetInputFloat&&) noexcept;
        ~WidgetInputFloat();

        f32 getValue() const;

        WidgetInputFloat& setOnChangedValueEvent(const OnWidgetEventFloatParam& event);

        TypePtr getType() const final;

        struct StateInputFloat : StateInputFieldBase
        {
            OnWidgetEventFloatParam _onChangedValueEvent;
        };

    private:

        using WidgetType = WidgetInputFloat;
        using StateType = StateInputFloat;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;

    };

    inline f32 WidgetInputFloat::getValue() const
    {
        return Widget::cast_data<StateType>(m_data)._value[0];
    }

    inline WidgetInputFloat& WidgetInputFloat::setOnChangedValueEvent(const OnWidgetEventFloatParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    inline TypePtr WidgetInputFloat::getType() const
    {
        return typeOf<WidgetInputFloat>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetInputFloat3 class
    */
    class WidgetInputFloat3 final : public WidgetInputFieldBase<WidgetInputFloat3, f32, 3>
    {
    public:

        explicit WidgetInputFloat3(f32 value0, f32 value1, f32 value2) noexcept;
        WidgetInputFloat3(const WidgetInputFloat3&) noexcept;
        WidgetInputFloat3(WidgetInputFloat3&&) noexcept;
        ~WidgetInputFloat3();

        std::array<f32, 3> getValue() const;

        WidgetInputFloat3& setOnChangedValueEvent(const OnWidgetEventFloat3Param& event);

        TypePtr getType() const final;

        struct StateInputFloat3 : StateInputFieldBase
        {
            OnWidgetEventFloat3Param _onChangedValueEvent;
        };

    private:

        using WidgetType = WidgetInputFloat3;
        using StateType = StateInputFloat3;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;

    };

    inline std::array<f32, 3> WidgetInputFloat3::getValue() const
    {
        return Widget::cast_data<StateType>(m_data)._value;
    }

    inline WidgetInputFloat3& WidgetInputFloat3::setOnChangedValueEvent(const OnWidgetEventFloat3Param& event)
    {
        Widget::cast_data<StateType>(m_data)._onChangedValueEvent = event;
        return *this;
    }

    inline TypePtr WidgetInputFloat3::getType() const
    {
        return typeOf<WidgetInputFloat3>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetInputText>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WidgetInputInt>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WidgetInputFloat>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WidgetInputFloat3>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
