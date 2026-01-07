#pragma once

#include "Widget.h"
#include "WidgetLayout.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetPopup wiget class
    */
    class WidgetPopup final : public WidgetBase<WidgetPopup>
    {
    public:

        enum PopupFlag
        {
            Moveable = 1 << 0,
            Resizeable = 1 << 1,
            Scrollable = 1 << 2,
            AutoResizeByContent = 1 << 3,
            Module = 1 << 4

        };
        typedef u64 PopupFlags;

        explicit WidgetPopup(const std::string& title, PopupFlags flags = 0) noexcept;
        explicit WidgetPopup(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos, PopupFlags flags = 0) noexcept;
        WidgetPopup(const WidgetPopup&) noexcept;
        WidgetPopup(WidgetPopup&&) noexcept;
        ~WidgetPopup();

        bool open(const std::string& args);
        bool close();

        bool isFocused() const;

        const math::Dimension2D& getSize() const;
        const math::Point2D& getPosition() const;
        const std::string& getTitle() const;

        WidgetPopup& setSize(const math::Dimension2D& size);
        WidgetPopup& setPosition(const math::Point2D& position);
        WidgetPopup& setTitle(const std::string& title);

        template<class TWidget>
        WidgetPopup& addWidget(const TWidget& wiget);

        template<class TWidget>
        WidgetPopup& addWidget(TWidget&& wiget);

        void removeWigets();

        TypePtr getType() const final;

        struct StatePopup : StateBase
        {
            math::Dimension2D             _size;
            math::Point2D                 _position;
            std::string                   _title;
            PopupFlags                    _createFlags = 0;
            WidgetLayout                  _layout;
        };

    private:

        using WidgetType = WidgetPopup;
        using StateType = StatePopup;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
        Widget* copy() const final;
    };

    inline bool WidgetPopup::isFocused() const
    {
        return Widget::cast_data<StateType>(m_data)._isFocused;
    }

    inline const math::Dimension2D& WidgetPopup::getSize() const
    {
        return Widget::cast_data<StateType>(m_data)._size;
    }

    inline const math::Point2D& WidgetPopup::getPosition() const
    {
        return  Widget::cast_data<StateType>(m_data)._position;
    }

    inline const std::string& WidgetPopup::getTitle() const
    {
        return  Widget::cast_data<StateType>(m_data)._title;
    }

    inline WidgetPopup& WidgetPopup::setSize(const math::Dimension2D& size)
    {
        Widget::cast_data<StateType>(m_data)._size = size;
        return *this;
    }

    inline WidgetPopup& WidgetPopup::setPosition(const math::Point2D& position)
    {
        Widget::cast_data<StateType>(m_data)._position = position;
        return *this;
    }

    inline WidgetPopup& WidgetPopup::setTitle(const std::string& title)
    {
        Widget::cast_data<StateType>(m_data)._title = title;
        return *this;
    }

    template<class TWidget>
    inline WidgetPopup& WidgetPopup::addWidget(const TWidget& wiget)
    {
        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget(wiget);
        return *this;
    }

    template<class TWidget>
    inline WidgetPopup& WidgetPopup::addWidget(TWidget&& wiget)
    {
        static_assert(std::is_move_constructible<TWidget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWidget>::value, "should be noexcept movable");

        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.addWidget(std::forward<TWidget>(wiget));
        return *this;
    }

    inline void WidgetPopup::removeWigets()
    {
        WidgetLayout& layout = Widget::cast_data<StateType>(m_data)._layout;
        layout.removeWigets();
    }

    inline TypePtr WidgetPopup::getType() const
    {
        return typeOf<WidgetType>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

template<>
struct TypeOf<ui::WidgetPopup>
{
    static TypePtr get()
    {
        static TypePtr ptr = nullptr;
        return (TypePtr)&ptr;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace v3d