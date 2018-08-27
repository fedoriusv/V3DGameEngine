#pragma once

#include "Common.h"
#include "Event/KeyCodes.h"

namespace v3d
{
namespace event
{
    class InputEventReceiver;

} //namespace event
namespace platform
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

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
        static Window*  createWindow(const core::Dimension2D& size = { 1024U, 768U }, const core::Point2D& pos = { 100U, 100U }, bool fullscreen = false, event::InputEventReceiver* receiver = nullptr);
        static bool     updateWindow(Window* window);
        static void     detroyWindow(Window* window);

        Window(const WindowParam& params, event::InputEventReceiver* receiver);
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

        event::InputEventReceiver* getInputEventReceiver() const;

    protected:

        virtual bool initialize() = 0;
        virtual bool update() = 0;
        virtual void destroy() = 0;

        WindowParam                 m_params;

        event::KeyCodes             m_keyCodes;
        event::InputEventReceiver*  m_receiver;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
