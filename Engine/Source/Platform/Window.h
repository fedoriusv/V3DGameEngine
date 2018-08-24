#pragma once

#include "Common.h"

namespace v3d
{
namespace platform
{

    class Window
    {
    public:

        struct WindowParam
        {
            std::wstring            _caption;
            core::Dimension2D       _size;
            core::Point2D           _position;
            bool                    _isFullscreen;
            bool                    _isResizable;
            bool                    _isVisible;
            bool                    _isMaximized;
            bool                    _isMinimized;
            bool                    _isActive;
            bool                    _isFocused;

            WindowParam()
                : _caption(L"Window")
                , _size(core::Dimension2D(1024U, 768U))
                , _position(core::Point2D(0U, 0U))
                , _isFullscreen(false)
                , _isResizable(false)
                , _isVisible(true)
                , _isMaximized(false)
                , _isMinimized(false)
                , _isActive(true)
                , _isFocused(true)
            {
            }
        };

        static Window*  createWindow(const core::Dimension2D& size = { 1024U, 768U }, const core::Point2D& pos = { 100U, 100U }, bool fullscreen = false, bool resizable = false);
        static bool     updateWindow(Window* window);
        static void     detroyWindow(Window* window);

        explicit Window(const WindowParam& params);
        virtual ~Window();

        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;

        virtual void minimize() = 0;
        virtual void maximize() = 0;
        virtual void restore() = 0;

        virtual void setFullScreen(bool value = true) = 0;
        virtual void setResizeble(bool value = true) = 0;
        virtual void setTextCaption(const std::string& text) = 0;
        virtual void setPosition(const core::Point2D& pos) = 0;

        virtual bool isMaximized() const = 0;
        virtual bool isMinimized() const = 0;
        virtual bool isActive() const = 0;
        virtual bool isFocused() const = 0;
        const core::Point2D& getPosition() const;

        bool isFullscreen() const;
        bool isResizable() const;
        const core::Dimension2D& getSize() const;

    protected:

        virtual bool initialize() = 0;
        virtual bool update() = 0;
        virtual void destroy() = 0;

        WindowParam m_params;

    };

} //namespace platform
} //namespace v3d
