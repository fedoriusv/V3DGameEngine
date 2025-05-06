#pragma once

#include "Common.h"
#include "Wiget.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WigetHandler;

    class WigetWindow;
    class WigetMenuBar;
    class WigetMenu;
    class WigetTabBar;

    class WigetLayout;
    class WigetHorizontalLayout;


    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetLayoutBase class
    */
    template<class TWigetLayout>
    class WigetLayoutBase : public WigetBase<TWigetLayout>
    {

    public:

        enum LayoutFlag
        {
            Border = 1 << 0,
            Fill = 1 << 1,
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

        TWigetLayout& setFontSize(FontSize size);
        TWigetLayout& setPadding(const math::RectF32& padding);
        TWigetLayout& setSize(const math::Dimension2D& size);
        TWigetLayout& setHAlignment(HorizontalAlignment align);
        TWigetLayout& setVAlignment(VerticalAlignment align);

        HorizontalAlignment getHAlignment() const;
        VerticalAlignment getVAlignment() const;

        template<class TWiget>
        TWigetLayout& addWiget(const TWiget& wiget);

        template<class TWiget>
        TWigetLayout& addWiget(TWiget&& wiget);

        Wiget* findWidgetByID(u64 id);

        struct StateLayoutBase : WigetBase<TWigetLayout>::StateBase
        {
            math::Dimension2D   _size;
            math::RectF32       _padding;
            LayoutFlags         _flags = 0;
            FontSize            _fontSize = FontSize::MediumFont;
            HorizontalAlignment _aligmentH = HorizontalAlignment::AlignmentLeft;
            VerticalAlignment   _aligmentV = VerticalAlignment::AlignmentTop;


            math::RectF32       _cachedLayoutRect;
            math::Vector2D      _cachedLayoutOffest;
            math::Vector2D      _cachedContentSize;
        };

    protected:

        explicit WigetLayoutBase(Wiget::State* state) noexcept;
        WigetLayoutBase(const WigetLayoutBase&) noexcept;
        WigetLayoutBase(WigetLayoutBase&&) noexcept;
        ~WigetLayoutBase() = default;

        std::vector<Wiget*> m_wigets;

    private:

        using StateType = StateLayoutBase;
    };

    template<class TWigetLayout>
    inline WigetLayoutBase<TWigetLayout>::WigetLayoutBase(Wiget::State* state) noexcept
        : WigetBase<TWigetLayout>(state)
    {
    }

    template<class TWigetLayout>
    inline WigetLayoutBase<TWigetLayout>::WigetLayoutBase(const WigetLayoutBase& other) noexcept
        : WigetBase<TWigetLayout>(other)
    {
    }

    template<class TWigetLayout>
    inline WigetLayoutBase<TWigetLayout>::WigetLayoutBase(WigetLayoutBase&& other) noexcept
        : WigetBase<TWigetLayout>(other)
    {
    }

    template<class TWigetLayout>
    inline TWigetLayout& WigetLayoutBase<TWigetLayout>::setFontSize(FontSize size)
    {
        Wiget::cast_data<StateType>(WigetBase<TWigetLayout>::m_data)._fontSize = size;
        return *static_cast<TWigetLayout*>(this);
    }

    template<class TWigetLayout>
    inline TWigetLayout& WigetLayoutBase<TWigetLayout>::setPadding(const math::RectF32& padding)
    {
        Wiget::cast_data<StateType>(WigetBase<TWigetLayout>::m_data)._padding = padding;
        return *static_cast<TWigetLayout*>(this);
    }

    template<class TWigetLayout>
    inline TWigetLayout& WigetLayoutBase<TWigetLayout>::setSize(const math::Dimension2D& size)
    {
        Wiget::cast_data<StateType>(WigetBase<TWigetLayout>::m_data)._size = size;
        return *static_cast<TWigetLayout*>(this);
    }

    template<class TWigetLayout>
    inline TWigetLayout& WigetLayoutBase<TWigetLayout>::setHAlignment(HorizontalAlignment align)
    {
        Wiget::cast_data<StateType>(WigetBase<TWigetLayout>::m_data)._aligmentH = align;
        return *static_cast<TWigetLayout*>(this);
    }

    template<class TWigetLayout>
    inline TWigetLayout& WigetLayoutBase<TWigetLayout>::setVAlignment(VerticalAlignment align)
    {
        Wiget::cast_data<StateType>(WigetBase<TWigetLayout>::m_data)._aligmentV = align;
        return *static_cast<TWigetLayout*>(this);
    }

    template<class TWigetLayout>
    inline WigetLayoutBase<TWigetLayout>::HorizontalAlignment WigetLayoutBase<TWigetLayout>::getHAlignment() const
    {
        return Wiget::cast_data<StateType>(WigetBase<TWigetLayout>::m_data)._aligmentH;
    }

    template<class TWigetLayout>
    inline WigetLayoutBase<TWigetLayout>::VerticalAlignment WigetLayoutBase<TWigetLayout>::getVAlignment() const
    {
        return Wiget::cast_data<StateType>(WigetBase<TWigetLayout>::m_data)._aligmentV;
    }

    template<class TWigetLayout>
    inline Wiget* WigetLayoutBase<TWigetLayout>::findWidgetByID(u64 id)
    {
        for (auto& wiget : m_wigets)
        {
            if (wiget->getID() == id)
            {
                return wiget;
            }

            if (wiget->getType() == type_of<WigetLayout>())
            {
                Wiget* child = static_cast<WigetLayout*>(wiget)->findWidgetByID(id);
                if (child)
                {
                    return child;
                }
            }
            else if (wiget->getType() == type_of<WigetHorizontalLayout>())
            {
                Wiget* child = static_cast<WigetHorizontalLayout*>(wiget)->findWidgetByID(id);
                if (child)
                {
                    return child;
                }
            }
        }

        return nullptr;
    }

    template<class TWigetLayout>
    template<class TWiget>
    inline TWigetLayout& WigetLayoutBase<TWigetLayout>::addWiget(const TWiget& wiget)
    {
        TWiget* obj = V3D_NEW(TWiget, memory::MemoryLabel::MemoryUI)(wiget);
        m_wigets.push_back(obj);
        return *static_cast<TWigetLayout*>(this);
    }

    template<class TWigetLayout>
    template<class TWiget>
    inline TWigetLayout& WigetLayoutBase<TWigetLayout>::addWiget(TWiget&& wiget)
    {
        static_assert(std::is_move_constructible<TWiget>::value, "must be movable");
        static_assert(std::is_nothrow_move_constructible<TWiget>::value, "should be noexcept movable");

        using TWigetRaw = std::decay_t<TWiget>;
        TWigetRaw* obj = V3D_NEW(TWigetRaw, memory::MemoryLabel::MemoryUI)(std::forward<TWigetRaw>(wiget));
        m_wigets.push_back(obj);
        return *static_cast<TWigetLayout*>(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetLayout class
    */
    class WigetLayout : public WigetLayoutBase<WigetLayout>
    {
    public:

        explicit WigetLayout(LayoutFlags flags = 0) noexcept;
        WigetLayout(const WigetLayout&) noexcept;
        WigetLayout(WigetLayout&&) noexcept;
        virtual ~WigetLayout();

        TypePtr getType() const override;

        struct StateLayout : StateLayoutBase
        {
        };

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;

    private:

        using WigetType = WigetLayout;
        using StateType = StateLayout;

        friend WigetWindow;
        friend WigetMenuBar;
        friend WigetMenu;
        friend WigetTabBar;

        Wiget* copy() const override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetHorizontalLayout class
    */
    class WigetHorizontalLayout final : public WigetLayout
    {
    public:

        WigetHorizontalLayout() noexcept;
        WigetHorizontalLayout(const WigetHorizontalLayout&) noexcept;
        WigetHorizontalLayout(WigetHorizontalLayout&&) noexcept;
        ~WigetHorizontalLayout();

        TypePtr getType() const final;

        struct StateHorizontalLayout : StateLayout
        {
        };

    private:

        using WigetType = WigetHorizontalLayout;
        using StateType = StateHorizontalLayout;

        Wiget* copy() const final;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetWindowLayout class
    */
    class WigetWindowLayout final : public WigetBase<WigetWindowLayout>
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
            SplitDir     _dir = SplitDir::Main;
            f32          _ratio = 0.f;
            WigetWindow* _window = nullptr;
        };

        WigetWindowLayout() noexcept;
        explicit WigetWindowLayout(WigetWindow* main, const std::vector<LayoutRule>& layouts) noexcept;
        WigetWindowLayout(const WigetWindowLayout&) noexcept;
        WigetWindowLayout(WigetWindowLayout&&) noexcept;
        ~WigetWindowLayout();

        WigetWindowLayout& operator=(const WigetWindowLayout&);
        WigetWindowLayout& operator=(WigetWindowLayout&&);

        WigetWindow* getWindow() const;
        const std::vector<LayoutRule>& getRules() const;

        TypePtr getType() const final;

        struct StateViewportLayout : StateBase
        {
            WigetWindow* _main = nullptr;
            std::vector<LayoutRule> _rules;
        };

    private:

        using WigetType = WigetWindowLayout;
        using StateType = StateViewportLayout;

        bool update(WigetHandler* handler, Wiget* parent, Wiget* layout, f32 dt) final;
        Wiget* copy() const override;
    };

    inline const std::vector<WigetWindowLayout::LayoutRule>& WigetWindowLayout::getRules() const
    {
        return Wiget::cast_data<StateType>(m_data)._rules;
    }

    inline WigetWindow* WigetWindowLayout::getWindow() const
    {
        return Wiget::cast_data<StateType>(m_data)._main;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace ui

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<ui::WigetLayout>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WigetHorizontalLayout>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };
    
    template<>
    struct TypeOf<ui::WigetWindowLayout>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d