#pragma once

#include "Wiget.h"

namespace v3d
{
namespace ui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WigetWindow wiget class
    */
    class WigetWindow : public WigetBase<WigetWindow>
    {
    public:

        enum WindowFlag
        {
            Moveable,
            Resizeable
        };

        typedef u32 WindowFlags;

        explicit WigetWindow(const std::string& title, const math::Dimension2D& size, const math::Point2D& pos, WindowFlags flags = 0) noexcept;
        WigetWindow(const WigetWindow&) noexcept;
        ~WigetWindow();

        const math::Dimension2D& getSize() const;
        const math::Point2D& getPosition() const;
        const std::string& getTitle() const;

        WigetWindow& setSize(const math::Dimension2D& size);
        WigetWindow& setPosition(const math::Point2D& position);
        WigetWindow& setTitle(const std::string& title);

        template<class TWiget>
        WigetWindow& addWiget(TWiget& wiget);

        struct ContextWindow : ContextBase
        {
            math::Dimension2D   _size;
            math::Point2D       _position;
            std::string         _title;
            WindowFlags         _flags;
            WigetLayout         _layout;
        };

        bool update(WigetHandler* handler, WigetLayout* layout, f32 dt) override;

    };

    inline const math::Dimension2D& WigetWindow::getSize() const
    {
        return Wiget::cast_data<ContextWindow>(m_data)._size;
    }

    inline const math::Point2D& WigetWindow::getPosition() const
    {
        return  Wiget::cast_data<ContextWindow>(m_data)._position;
    }

    inline const std::string& WigetWindow::getTitle() const
    {
        return  Wiget::cast_data<ContextWindow>(m_data)._title;
    }

    inline WigetWindow& WigetWindow::setSize(const math::Dimension2D& size)
    {
        Wiget::cast_data<ContextWindow>(m_data)._size = size;
        return *this;
    }

    inline WigetWindow& WigetWindow::setPosition(const math::Point2D& position)
    {
        Wiget::cast_data<ContextWindow>(m_data)._position = position;
        return *this;
    }

    inline WigetWindow& WigetWindow::setTitle(const std::string& title)
    {
        Wiget::cast_data<ContextWindow>(m_data)._title = title;
        return *this;
    }

    template<class TWiget>
    inline WigetWindow& WigetWindow::addWiget(TWiget& wiget)
    {
        WigetLayout& layout = Wiget::cast_data<ContextWindow>(m_data)._layout;
        layout.addWiget(wiget);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ui
} // namespace v3d