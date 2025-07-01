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
    * @brief WidgetTreeNode class
    */
    class WidgetTreeNode final : public WidgetBase<WidgetTreeNode>
    {
    public:

        explicit WidgetTreeNode(const std::string& text) noexcept;
        WidgetTreeNode(const WidgetTreeNode&) noexcept;
        WidgetTreeNode(WidgetTreeNode&&) noexcept;
        ~WidgetTreeNode();

        const std::string& getText() const;
        WidgetTreeNode& setText(const std::string& text);

        template<class TWidget>
        WidgetTreeNode& addWidget(const TWidget& wiget);

        template<class TWidget>
        WidgetTreeNode& addWidget(TWidget&& wiget);

        void removeWigets();

        TypePtr getType() const final;

        struct StateTreeNode : StateBase
        {
            std::string         _text;
            WidgetLayout        _layout;
            bool                _collapsingHeader = true;
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

    inline TypePtr WidgetTreeNode::getType() const
    {
        return typeOf<WidgetType>();
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