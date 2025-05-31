#pragma once

#include "Common.h"
#include "Wiget.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetGroupBase interface
    */
    template<class TWiget>
    class WigetGroupBase : public WigetBase<TWiget>
    {
    protected:

        explicit WigetGroupBase(Wiget::State* state) noexcept;
        WigetGroupBase(const WigetGroupBase&) noexcept;
        ~WigetGroupBase() = default;

    public:

        TWiget& addElement(const std::string& text);
        TWiget& setActiveIndex(u32 index);

        u32 getActiveIndex() const;
        const std::string& getElement(u32 index) const;

        TWiget& setOnChangedIndexEvent(const OnWigetEventIntParam& event);

        struct StateWigetGroupBase : WigetBase<TWiget>::StateBase
        {
            std::vector<std::string> _list;
            OnWigetEventIntParam     _onChangedIndexEvent;
            u32                      _activeIndex;
        };

    private:

        using WigetType = WigetGroupBase;
        using StateType = StateWigetGroupBase;
    };

    template<class TWiget>
    inline WigetGroupBase<TWiget>::WigetGroupBase(Wiget::State* state) noexcept
        : WigetBase<TWiget>(state)
    {
    }

    template<class TWiget>
    inline WigetGroupBase<TWiget>::WigetGroupBase(const WigetGroupBase& other) noexcept
        : WigetBase<TWiget>(other)
    {
    }

    template<class TWiget>
    inline TWiget& WigetGroupBase<TWiget>::addElement(const std::string& text)
    {
        Wiget::cast_data<StateType>(WigetBase<TWiget>::m_data)._list.push_back(text);
        return *static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget& WigetGroupBase<TWiget>::setActiveIndex(u32 index)
    {
        Wiget::cast_data<StateType>(WigetBase<TWiget>::m_data)._activeIndex = index;
        return *static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline u32 WigetGroupBase<TWiget>::getActiveIndex() const
    {
        return Wiget::cast_data<StateType>(WigetBase<TWiget>::m_data)._activeIndex;
    }

    template<class TWiget>
    inline const std::string& WigetGroupBase<TWiget>::getElement(u32 index) const
    {
        ASSERT(index < Wiget::cast_data<StateWigetGroupBase>(WigetBase<TWiget>::m_data)._list.size(), "range out");
        return Wiget::cast_data<StateType>(WigetBase<TWiget>::m_data)._list[index];
    }

    template<class TWiget>
    inline TWiget& WigetGroupBase<TWiget>::setOnChangedIndexEvent(const OnWigetEventIntParam& event)
    {
        Wiget::cast_data<StateType>(WigetBase<TWiget>::m_data)._onChangedIndexEvent = event;
        return *static_cast<TWiget*>(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetRadioButtonGroup class
    */
    class WigetRadioButtonGroup final : public WigetGroupBase<WigetRadioButtonGroup>
    {
    public:

        explicit WigetRadioButtonGroup(std::vector<std::string>& buttons) noexcept;
        WigetRadioButtonGroup() noexcept;
        WigetRadioButtonGroup(const WigetRadioButtonGroup&) noexcept;
        ~WigetRadioButtonGroup();

        TypePtr getType() const final;

        struct StateRadioButtonGroup : StateWigetGroupBase
        {
        };

    private:

        using WigetType = WigetRadioButtonGroup;
        using StateType = StateRadioButtonGroup;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline TypePtr WigetRadioButtonGroup::getType() const
    {
        return typeOf<WigetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetComboBox class
    */
    class WigetComboBox final : public WigetGroupBase<WigetComboBox>
    {
    public:

        explicit WigetComboBox() noexcept;
        WigetComboBox(const WigetComboBox&) noexcept;
        ~WigetComboBox();

        TypePtr getType() const final;

        struct StateComboBox : StateWigetGroupBase
        {
        };

    private:

        using WigetType = WigetComboBox;
        using StateType = StateComboBox;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline TypePtr WigetComboBox::getType() const
    {
        return typeOf<WigetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetListBox class
    */
    class WigetListBox final : public WigetGroupBase<WigetListBox>
    {
    public:

        explicit WigetListBox() noexcept;
        WigetListBox(const WigetListBox&) noexcept;
        ~WigetListBox();

        TypePtr getType() const final;

        struct StateListBox : StateWigetGroupBase
        {
        };

    private:

        using WigetType = WigetListBox;
        using StateType = StateListBox;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const final;
    };

    inline TypePtr WigetListBox::getType() const
    {
        return typeOf<WigetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WigetRadioButtonGroup>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WigetComboBox>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    template<>
    struct TypeOf<ui::WigetListBox>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d