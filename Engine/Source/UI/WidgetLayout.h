#pragma once

#include "Common.h"
#include "Widget.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WidgetHandler;

    class WidgetWindow;
    class WidgetMenuBar;
    class WidgetMenu;
    class WidgetTreeNode;

    class WidgetLayout;
    class WidgetHorizontalLayout;


    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetLayoutBase class
    */
    template<class TWidgetLayout>
    class WidgetLayoutBase : public WidgetBase<TWidgetLayout>
    {

    public:

        enum LayoutFlag
        {
            Border = 1 << 0,
            Fill = 1 << 1,
            Test = 1 << 2,
        };
        typedef u32 LayoutFlags;

        enum FontSize : u32
        {
            SmallFont,
            MediumFont,
            LargeFont,

            FontSize_Count
        };

        enum class HorizontalAlignment : u32
        {
            AlignmentLeft,
            AlignmentCenter,
            AlignmentRight,
            AlignmentFill
        };

        enum class VerticalAlignment : u32
        {
            AlignmentTop,
            AlignmentCenter,
            AlignmentBottom,
            AlignmentFill
        };

        TWidgetLayout& setFontSize(FontSize size);
        TWidgetLayout& setPadding(const math::Rect& padding);
        TWidgetLayout& setSize(const math::Dimension2D& size);
        TWidgetLayout& setHAlignment(HorizontalAlignment align);
        TWidgetLayout& setVAlignment(VerticalAlignment align);

        HorizontalAlignment getHAlignment() const;
        VerticalAlignment getVAlignment() const;

        template<class TWidget>
        TWidgetLayout& addWidget(const TWidget& wiget);

        template<class TWidget>
        TWidgetLayout& addWidget(TWidget&& wiget);

        void removeWigets();

        Widget* findWidgetByID(u64 id);

        struct StateLayoutBase : WidgetBase<TWidgetLayout>::StateBase
        {
            math::Dimension2D    _size;
            math::Rect           _padding;
            LayoutFlags          _flags = 0;
            FontSize             _fontSize = FontSize::MediumFont;
            HorizontalAlignment  _aligmentH = HorizontalAlignment::AlignmentLeft;
            VerticalAlignment    _aligmentV = VerticalAlignment::AlignmentTop;


            math::Rect   _cachedLayoutRect;
            math::float2 _cachedLayoutOffest;
        };

    protected:

        explicit WidgetLayoutBase(Widget::State* state) noexcept;
        WidgetLayoutBase(const WidgetLayoutBase&) noexcept;
        WidgetLayoutBase(WidgetLayoutBase&&) noexcept;
        ~WidgetLayoutBase() = default;

        std::vector<Widget*> m_wigets;

    private:

        using StateType = StateLayoutBase;
    };

    template<class TWidgetLayout>
    inline WidgetLayoutBase<TWidgetLayout>::WidgetLayoutBase(Widget::State* state) noexcept
        : WidgetBase<TWidgetLayout>(state)
    {
    }

    template<class TWidgetLayout>
    inline WidgetLayoutBase<TWidgetLayout>::WidgetLayoutBase(const WidgetLayoutBase& other) noexcept
        : WidgetBase<TWidgetLayout>(other)
    {
    }

    template<class TWidgetLayout>
    inline WidgetLayoutBase<TWidgetLayout>::WidgetLayoutBase(WidgetLayoutBase&& other) noexcept
        : WidgetBase<TWidgetLayout>(other)
    {
    }

    template<class TWidgetLayout>
    inline TWidgetLayout& WidgetLayoutBase<TWidgetLayout>::setFontSize(FontSize size)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidgetLayout>::m_data)._fontSize = size;
        return *static_cast<TWidgetLayout*>(this);
    }

    template<class TWidgetLayout>
    inline TWidgetLayout& WidgetLayoutBase<TWidgetLayout>::setPadding(const math::Rect& padding)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidgetLayout>::m_data)._padding = padding;
        return *static_cast<TWidgetLayout*>(this);
    }

    template<class TWidgetLayout>
    inline TWidgetLayout& WidgetLayoutBase<TWidgetLayout>::setSize(const math::Dimension2D& size)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidgetLayout>::m_data)._size = size;
        Widget::cast_data<StateType>(WidgetBase<TWidgetLayout>::m_data)._stateMask |= Widget::State::StateMask::DefindedSize;
        return *static_cast<TWidgetLayout*>(this);
    }

    template<class TWidgetLayout>
    inline TWidgetLayout& WidgetLayoutBase<TWidgetLayout>::setHAlignment(HorizontalAlignment align)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidgetLayout>::m_data)._aligmentH = align;
        return *static_cast<TWidgetLayout*>(this);
    }

    template<class TWidgetLayout>
    inline TWidgetLayout& WidgetLayoutBase<TWidgetLayout>::setVAlignment(VerticalAlignment align)
    {
        Widget::cast_data<StateType>(WidgetBase<TWidgetLayout>::m_data)._aligmentV = align;
        return *static_cast<TWidgetLayout*>(this);
    }

    template<class TWidgetLayout>
    inline WidgetLayoutBase<TWidgetLayout>::HorizontalAlignment WidgetLayoutBase<TWidgetLayout>::getHAlignment() const
    {
        return Widget::cast_data<StateType>(WidgetBase<TWidgetLayout>::m_data)._aligmentH;
    }

    template<class TWidgetLayout>
    inline WidgetLayoutBase<TWidgetLayout>::VerticalAlignment WidgetLayoutBase<TWidgetLayout>::getVAlignment() const
    {
        return Widget::cast_data<StateType>(WidgetBase<TWidgetLayout>::m_data)._aligmentV;
    }

    template<class TWidgetLayout>
    inline Widget* WidgetLayoutBase<TWidgetLayout>::findWidgetByID(u64 id)
    {
        for (auto& wiget : m_wigets)
        {
            if (wiget->getID() == id)
            {
                return wiget;
            }

            if (wiget->getType() == typeOf<WidgetLayout>())
            {
                Widget* child = static_cast<WidgetLayout*>(wiget)->findWidgetByID(id);
                if (child)
                {
                    return child;
                }
            }
            else if (wiget->getType() == typeOf<WidgetHorizontalLayout>())
            {
                Widget* child = static_cast<WidgetHorizontalLayout*>(wiget)->findWidgetByID(id);
                if (child)
                {
                    return child;
                }
            }
        }

        return nullptr;
    }

    template<class TWidgetLayout>
    template<class TWidget>
    inline TWidgetLayout& WidgetLayoutBase<TWidgetLayout>::addWidget(const TWidget& wiget)
    {
        TWidget* obj = V3D_NEW(TWidget, memory::MemoryLabel::MemoryUI)(wiget);
        m_wigets.push_back(obj);
        return *static_cast<TWidgetLayout*>(this);
    }

    template<class TWidgetLayout>
    template<class TWidget>
    inline TWidgetLayout& WidgetLayoutBase<TWidgetLayout>::addWidget(TWidget&& wiget)
    {
        static_assert(std::is_move_constructible<TWidget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWidget>::value, "should be noexcept movable");

        using TWidgetRaw = std::decay_t<TWidget>;
        TWidgetRaw* obj = V3D_NEW(TWidgetRaw, memory::MemoryLabel::MemoryUI)(std::forward<TWidgetRaw>(wiget));
        m_wigets.push_back(obj);
        return *static_cast<TWidgetLayout*>(this);
    }

    template<class TWidgetLayout>
    inline void WidgetLayoutBase<TWidgetLayout>::removeWigets()
    {
        for (auto& wiget : m_wigets)
        {
            V3D_DELETE(wiget, memory::MemoryLabel::MemoryUI);
        }
        m_wigets.clear();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetLayout class
    */
    class WidgetLayout : public WidgetLayoutBase<WidgetLayout>
    {
    public:

        explicit WidgetLayout(LayoutFlags flags = 0) noexcept;
        WidgetLayout(const WidgetLayout&) noexcept;
        WidgetLayout(WidgetLayout&&) noexcept;
        virtual ~WidgetLayout();

        TypePtr getType() const override;

        struct StateLayout : StateLayoutBase
        {
        };

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;

    private:

        using WidgetType = WidgetLayout;
        using StateType = StateLayout;

        friend WidgetWindow;
        friend WidgetMenuBar;
        friend WidgetMenu;
        friend WidgetTreeNode;

        Widget* copy() const override;
        math::float2 calculateSize(WidgetHandler* handler, Widget* parent, Widget* layout) final;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetHorizontalLayout class
    */
    class WidgetHorizontalLayout final : public WidgetLayout
    {
    public:

        WidgetHorizontalLayout() noexcept;
        WidgetHorizontalLayout(const WidgetHorizontalLayout&) noexcept;
        WidgetHorizontalLayout(WidgetHorizontalLayout&&) noexcept;
        ~WidgetHorizontalLayout();

        TypePtr getType() const final;

        struct StateHorizontalLayout : StateLayout
        {
        };

    private:

        using WidgetType = WidgetHorizontalLayout;
        using StateType = StateHorizontalLayout;

        Widget* copy() const final;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WidgetWindowLayout class
    */
    class WidgetWindowLayout final : public WidgetBase<WidgetWindowLayout>
    {
    public:

        enum SplitDir : s32
        {
            Main = -1,
            DirLeft,
            DirRight,
            DirUp,
            DirDown
        };

        struct LayoutRule
        {
            SplitDir      _dir = SplitDir::Main;
            f32           _ratio = 0.f;
            WidgetWindow* _window = nullptr;
        };

        WidgetWindowLayout() noexcept;
        explicit WidgetWindowLayout(WidgetWindow* main, const std::vector<LayoutRule>& layouts) noexcept;
        WidgetWindowLayout(const WidgetWindowLayout&) noexcept;
        WidgetWindowLayout(WidgetWindowLayout&&) noexcept;
        ~WidgetWindowLayout();

        WidgetWindowLayout& operator=(const WidgetWindowLayout&);
        WidgetWindowLayout& operator=(WidgetWindowLayout&&);

        WidgetWindow* getWindow() const;
        const std::vector<LayoutRule>& getRules() const;

        TypePtr getType() const final;

        struct StateViewportLayout : StateBase
        {
            WidgetWindow* _main = nullptr;
            std::vector<LayoutRule> _rules;
        };

    private:

        using WidgetType = WidgetWindowLayout;
        using StateType = StateViewportLayout;

        bool update(WidgetHandler* handler, Widget* parent, Widget* layout, f32 dt) final;
        Widget* copy() const override;
    };

    inline const std::vector<WidgetWindowLayout::LayoutRule>& WidgetWindowLayout::getRules() const
    {
        return Widget::cast_data<StateType>(m_data)._rules;
    }

    inline WidgetWindow* WidgetWindowLayout::getWindow() const
    {
        return Widget::cast_data<StateType>(m_data)._main;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WidgetLayout>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WidgetHorizontalLayout>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WidgetWindowLayout>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d