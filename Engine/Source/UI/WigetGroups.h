#pragma once

#include "Common.h"
#include "Wiget.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetGroup interface
    */
    template<class TWiget>
    class WigetGroup : public WigetBase<TWiget>
    {
    protected:

        explicit WigetGroup(Wiget::Context* context) noexcept;
        WigetGroup(const WigetGroup&) noexcept;
        ~WigetGroup() = default;

    public:

        TWiget& addElement(const std::string& text);
        TWiget& setActiveIndex(u32 index);

        u32 getActiveIndex() const;
        const std::string& getElement(u32 index) const;

        TWiget& setOnChangedIndexEvent(const OnWigetEventIntParam& event);

        struct ContextWigetGroup : WigetBase<TWiget>::ContextBase
        {
            std::vector<std::string> _list;
            OnWigetEventIntParam     _onChangedIndexEvent;
            u32                      _activeIndex;
        };
    };

    template<class TWiget>
    inline WigetGroup<TWiget>::WigetGroup(Wiget::Context* context) noexcept
        : WigetBase<TWiget>(context)
    {
    }

    template<class TWiget>
    inline WigetGroup<TWiget>::WigetGroup(const WigetGroup& other) noexcept
        : WigetBase<TWiget>(other)
    {
    }

    template<class TWiget>
    inline TWiget& WigetGroup<TWiget>::addElement(const std::string& text)
    {
        Wiget::cast_data<ContextWigetGroup>(WigetBase<TWiget>::m_data)._list.push_back(text);
        return *static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline TWiget& WigetGroup<TWiget>::setActiveIndex(u32 index)
    {
        Wiget::cast_data<ContextWigetGroup>(WigetBase<TWiget>::m_data)._activeIndex = index;
        return *static_cast<TWiget*>(this);
    }

    template<class TWiget>
    inline u32 WigetGroup<TWiget>::getActiveIndex() const
    {
        return Wiget::cast_data<ContextWigetGroup>(WigetBase<TWiget>::m_data)._activeIndex;
    }

    template<class TWiget>
    inline const std::string& WigetGroup<TWiget>::getElement(u32 index) const
    {
        ASSERT(index < Wiget::cast_data<ContextWigetGroup>(WigetBase<TWiget>::m_data)._list.size(), "range out");
        return Wiget::cast_data<ContextWigetGroup>(WigetBase<TWiget>::m_data)._list[index];
    }

    template<class TWiget>
    inline TWiget& WigetGroup<TWiget>::setOnChangedIndexEvent(const OnWigetEventIntParam& event)
    {
        Wiget::cast_data<ContextWigetGroup>(WigetBase<TWiget>::m_data)._onChangedIndexEvent = event;
        return *static_cast<TWiget*>(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetRadioButtonGroup class
    */
    class WigetRadioButtonGroup : public WigetGroup<WigetRadioButtonGroup>
    {
    public:

        explicit WigetRadioButtonGroup(std::vector<std::string>& buttons) noexcept;
        WigetRadioButtonGroup() noexcept;
        WigetRadioButtonGroup(const WigetRadioButtonGroup&) noexcept;
        ~WigetRadioButtonGroup();

        struct ContextRadioButtonGroup : ContextWigetGroup
        {
        };

    private:

        bool update(WigetHandler* handler, Wiget* parent, WigetLayout* layout, f32 dt) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetComboBox class
    */
    class WigetComboBox : public WigetGroup<WigetComboBox>
    {
    public:

        explicit WigetComboBox() noexcept;
        WigetComboBox(const WigetComboBox&) noexcept;
        ~WigetComboBox();

        struct ContextComboBox : ContextWigetGroup
        {
        };

    private:

        bool update(WigetHandler* handler, Wiget* parent, WigetLayout* layout, f32 dt) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetListBox class
    */
    class WigetListBox : public WigetGroup<WigetListBox>
    {
    public:

        explicit WigetListBox() noexcept;
        WigetListBox(const WigetListBox&) noexcept;
        ~WigetListBox();

        struct ContextListBox : ContextWigetGroup
        {
        };

    private:

        bool update(WigetHandler* handler, Wiget* parent, WigetLayout* layout, f32 dt) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d