#pragma once

#include "Common.h"
#include "Widget.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using OnWidgetEventNode = std::function<void(Widget*, s32, bool)>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetTreeNode class
    */
    class WidgetTreeNode final : public WidgetBase<WidgetTreeNode>
    {
    public:

        enum TreeNodeFlag
        {
            Framed = 1 << 0,
            Open = 1 << 1,
            NoCollapsed = 1 << 2,
        };
        typedef u64 TreeNodeFlags;

        explicit WidgetTreeNode(const std::string& text, TreeNodeFlags flags = 0) noexcept;
        WidgetTreeNode(const WidgetTreeNode&) noexcept;
        WidgetTreeNode(WidgetTreeNode&&) noexcept;
        ~WidgetTreeNode();

        const std::string& getText() const;
        WidgetTreeNode& setText(const std::string& text);

        WidgetTreeNode& setIndex(u32 index);
        WidgetTreeNode& setUserData(void* userdata);

        WidgetTreeNode& setSelected(bool selected);
        WidgetTreeNode& setOnSelectChanged(const OnWidgetEventNode& event);

        WidgetTreeNode& setOnClickEvent(const OnWidgetEventIntParam& event);
        WidgetTreeNode& setOnClickEvent(const OnWidgetEventVoidParam& event);

        template<class TWidget>
        WidgetTreeNode& addWidget(const TWidget& wiget);

        template<class TWidget>
        WidgetTreeNode& addWidget(TWidget&& wiget);

        void removeWigets();

        TypePtr getType() const final;

        struct StateTreeNode : StateBase
        {
            std::string              _text;
            WidgetLayout             _layout;
            TreeNodeFlags            _createFlags = 0;
            OnWidgetEventNode        _onSelectedChanged;
            OnWidgetEventIntParam    _onClickEvent;
            OnWidgetEventVoidParam   _onClickEventUserData;
            void*                    _userdata = nullptr;
            u32                      _index = ~1;
            bool                     _isSelected;
        };

    private:

        using WidgetType = WidgetTreeNode;
        using StateType = StateTreeNode;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;
    };

    inline const std::string& WidgetTreeNode::getText() const
    {
        return Widget::cast_data<StateType>(m_data)._text;
    }

    inline WidgetTreeNode& WidgetTreeNode::setText(const std::string& text)
    {
        Widget::cast_data<StateType>(m_data)._text = text;
        return *this;
    }

    inline WidgetTreeNode& WidgetTreeNode::setIndex(u32 index)
    {
        Widget::cast_data<StateType>(m_data)._index = index;
        return *this;
    }

    inline WidgetTreeNode& WidgetTreeNode::setUserData(void* userdata)
    {
        Widget::cast_data<StateType>(m_data)._userdata = userdata;
        return *this;
    }

    inline WidgetTreeNode& WidgetTreeNode::setSelected(bool selected)
    {
        if (cast_data<StateType>(m_data)._isSelected != selected)
        {
            cast_data<StateType>(m_data)._isSelected = selected;
            cast_data<StateType>(m_data)._stateMask = State::StateMask::Selected;
        }
        return *this;
    }

    inline WidgetTreeNode& WidgetTreeNode::setOnSelectChanged(const OnWidgetEventNode& event)
    {
        Widget::cast_data<StateType>(m_data)._onSelectedChanged = event;
        return *this;
    }

    inline WidgetTreeNode& WidgetTreeNode::setOnClickEvent(const OnWidgetEventIntParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onClickEvent = event;
        return *this;
    }

    inline WidgetTreeNode& WidgetTreeNode::setOnClickEvent(const OnWidgetEventVoidParam& event)
    {
        Widget::cast_data<StateType>(m_data)._onClickEventUserData = event;
        return *this;
    }

    inline TypePtr WidgetTreeNode::getType() const
    {
        return typeOf<WidgetTreeNode>();
    }

    template<class TWidget>
    inline WidgetTreeNode& WidgetTreeNode::addWidget(const TWidget& wiget)
    {
        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget(wiget);
        return *this;
    }

    template<class TWidget>
    inline WidgetTreeNode& WidgetTreeNode::addWidget(TWidget&& wiget)
    {
        static_assert(std::is_move_constructible<TWidget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWidget>::value, "should be noexcept movable");

        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget(std::forward<TWidget>(wiget));
        return *this;
    }

    inline void WidgetTreeNode::removeWigets()
    {
        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.removeWigets();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetTreeNode>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d