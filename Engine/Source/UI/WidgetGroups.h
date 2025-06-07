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
    template<class TWidget>
    class WidgetGroupBase : public WidgetBase<TWidget>
    {
    protected:

        explicit WidgetGroupBase(Widget::State* state) noexcept;
        WidgetGroupBase(const WidgetGroupBase&) noexcept;
        ~WidgetGroupBase() = default;

    public:

        TWidget& addElement(const std::string& text);
        TWidget& setActiveIndex(u32 index);

        u32 getActiveIndex() const;
        const std::string& getElement(u32 index) const;

        TWidget& setOnChangedIndexEvent(const OnWidgetEventIntParam& event);

        struct StateWidgetGroupBase : WidgetBase<TWidget>::StateBase
        {
            std::vector<std::string> _list;
            OnWidgetEventIntParam    _onChangedIndexEvent;
            u32                      _activeIndex;
        };

    private:

        using WidgetType = WidgetGroupBase;
        using StateType = StateWidgetGroupBase;
    };

    template<class TWidget>
    inline WidgetGroupBase<TWidget>::WidgetGroupBase(Widget::State* state) noexcept
        : WidgetBase<TWidget>(state)
    {
    }

    template<class TWidget>
    inline WidgetGroupBase<TWidget>::WidgetGroupBase(const WidgetGroupBase& other) noexcept
        : WidgetBase<TWidget>(other)
    {
    }

    template<class TWidget>
    inline TWidget& WidgetGroupBase<TWidget>::addElement(const std::string& text)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._list.push_back(text);
        return *static_cast<TWidget*>(this);
    }

    template<class TWidget>
    inline TWidget& WidgetGroupBase<TWidget>::setActiveIndex(u32 index)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._activeIndex = index;
        return *static_cast<TWidget*>(this);
    }

    template<class TWidget>
    inline u32 WidgetGroupBase<TWidget>::getActiveIndex() const
    {
        return Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._activeIndex;
    }

    template<class TWidget>
    inline const std::string& WidgetGroupBase<TWidget>::getElement(u32 index) const
    {
        ASSERT(index < Widget::cast_data<StateWidgetGroupBase>(WidgetBase<TWidget>::m_data)._list.size(), "range out");
        return Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._list[index];
    }

    template<class TWidget>
    inline TWidget& WidgetGroupBase<TWidget>::setOnChangedIndexEvent(const OnWidgetEventIntParam& event)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidget>::m_data)._onChangedIndexEvent = event;
        return *static_cast<TWidget*>(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetRadioButtonGroup class
    */
    class WidgetRadioButtonGroup final : public WidgetGroupBase<WidgetRadioButtonGroup>
    {
    public:

        explicit WidgetRadioButtonGroup(std::vector<std::string>& buttons) noexcept;
        WidgetRadioButtonGroup() noexcept;
        WidgetRadioButtonGroup(const WidgetRadioButtonGroup&) noexcept;
        ~WidgetRadioButtonGroup();

        TypePtr getType() const final;

        struct StateRadioButtonGroup : StateWidgetGroupBase
        {
        };

    private:

        using WidgetType = WidgetRadioButtonGroup;
        using StateType = StateRadioButtonGroup;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline TypePtr WidgetRadioButtonGroup::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetComboBox class
    */
    class WidgetComboBox final : public WidgetGroupBase<WidgetComboBox>
    {
    public:

        explicit WidgetComboBox() noexcept;
        WidgetComboBox(const WidgetComboBox&) noexcept;
        ~WidgetComboBox();

        TypePtr getType() const final;

        struct StateComboBox : StateWidgetGroupBase
        {
        };

    private:

        using WidgetType = WidgetComboBox;
        using StateType = StateComboBox;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline TypePtr WidgetComboBox::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetListBox class
    */
    class WidgetListBox final : public WidgetGroupBase<WidgetListBox>
    {
    public:

        explicit WidgetListBox() noexcept;
        WidgetListBox(const WidgetListBox&) noexcept;
        ~WidgetListBox();

        TypePtr getType() const final;

        struct StateListBox : StateWidgetGroupBase
        {
        };

    private:

        using WidgetType = WidgetListBox;
        using StateType = StateListBox;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const final;
    };

    inline TypePtr WidgetListBox::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetRadioButtonGroup>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WidgetComboBox>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WidgetListBox>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d