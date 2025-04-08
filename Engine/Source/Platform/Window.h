#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Utils/ResourceID.h"
#include "Events/KeyCodes.h"

namespace v3d
{
namespace event
{
    class InputEventReceiver;

} //namespace event
namespace platform
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Window class. Base class of window
    */
    class V3D_API Window : public utils::Observable, public utils::ResourceID<Window, u32>
    {
    public:

        /**
        * @brief WindowParams struct
        */
        struct WindowParams
        {
            std::wstring            _name;
            math::Dimension2D       _size;
            math::Point2D           _position;
            bool                    _isFullscreen;
            bool                    _isResizable;
            bool                    _isVisible;
            bool                    _isMaximized;
            bool                    _isMinimized;
            bool                    _isActive;
            bool                    _isFocused;

            WindowParams() noexcept
                : _name(L"Window")
                , _size(math::Dimension2D(1024U, 768U))
                , _position(math::Point2D(0U, 0U))
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

        /**
        * @brief createWindow function. Create new window.
        * @param const math::Dimension2D& size [required]
        * @param const math::Point2D& pos [required]
        * @param bool fullscreen [optional]
        * @param bool resizable [optional]
        * @param InputEventReceiver* receiver [optional]
        * @param const std::wstring& name [optional]
        * @return pointer of created window
        */
        [[nodiscard]] static Window* createWindow(const math::Dimension2D& size, const math::Point2D& pos, bool fullscreen = false, bool resizable = false, event::InputEventReceiver* receiver = nullptr , [[maybe_unused]] const std::wstring& name = L"");

        /**
        * @brief createWindow function. Create new window.
        * @param const math::Dimension2D& size [required]
        * @param const math::Point2D& pos [required]
        * @param bool fullscreen [optional]
        * @param event::InputEventReceiver* receiver [optional]
        * @param const std::wstring& name [optional]
        * @return pointer of created window
        */
        [[nodiscard]] static Window* createWindow(const math::Dimension2D& size, const math::Point2D& pos, bool fullscreen = false, event::InputEventReceiver* receiver = nullptr, [[maybe_unused]] const std::wstring& name = L"");

        /**
        * @brief createWindow function. Create new window.
        * @param const math::Dimension2D& size [required]
        * @param const math::Point2D& pos [required]
        * @param const Window* parent [required]
        * @param bool resizable [optional]
        * @param const std::wstring& name [optional]
        * @return pointer of created window
        */
        [[nodiscard]] static Window* createWindow(const math::Dimension2D& size, const math::Point2D& pos, Window* parent, bool resizable = false, [[maybe_unused]] const std::wstring& name = L"");

        /**
        * @brief updateWindow function. Updates window
        * @param Window* window [required]
        */
        static bool updateEvents(Window* window);

        /**
        * @brief detroyWindow function. Removes window
        * @param Window* window [required]
        */
        static void detroyWindow(Window* window);

        virtual void minimize() = 0;
        virtual void maximize() = 0;
        virtual void restore() = 0;
        virtual void show() = 0;
        virtual void focus() = 0;

        virtual void setFullScreen(bool value = true) = 0;
        virtual void setResizeble(bool value = true) = 0;
        virtual void setText(const std::wstring& text) = 0;
        virtual void setSize(const math::Dimension2D& size) = 0;
        virtual void setPosition(const math::Point2D& pos) = 0;


        virtual bool isMaximized() const = 0;
        virtual bool isMinimized() const = 0;
        virtual bool isActive() const = 0;
        virtual bool isFocused() const = 0;

        virtual bool isFullscreen() const;
        virtual bool isResizable() const;

        virtual const math::Dimension2D& getSize() const;
        virtual const math::Point2D& getPosition() const;
        virtual const std::wstring& getText() const;

        event::InputEventReceiver* getInputEventReceiver() const;

        virtual NativeInstance getInstance() const = 0;
        virtual NativeWindow getWindowHandle() const = 0;

        virtual bool isValid() const = 0;

        static Window* getWindowsByID(u32 id);

    protected:

        /**
        * @brief Window constructor
        * @param const WindowParams& params [required]
        * @param event::InputEventReceiver* receiver [required]
        */
        explicit Window(const WindowParams& params, event::InputEventReceiver* receiver) noexcept;

        /**
        * @brief Window destructor
        */
        virtual ~Window();

        Window() = delete;
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        virtual bool initialize() = 0;
        virtual bool update() = 0;
        virtual void destroy() = 0;

        mutable WindowParams m_params;

        event::KeyCodes m_keyCodes;
        event::InputEventReceiver* m_receiver;

        static std::map<u32, Window*> s_windowsList;

        template<class T>
        friend void memory::internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line);

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline bool Window::isFullscreen() const
    {
        return m_params._isFullscreen;
    }

    inline bool Window::isResizable() const
    {
        return m_params._isResizable;
    }

    inline const math::Dimension2D& Window::getSize() const
    {
        return m_params._size;
    }

    inline const math::Point2D& Window::getPosition() const
    {
        return m_params._position;
    }

    inline const std::wstring& Window::getText() const
    {
        return m_params._name;
    }

    inline Window* Window::getWindowsByID(u32 id)
    {
        auto found = s_windowsList.find(id);
        if (found != s_windowsList.cend())
        {
            return found->second;
        }

        return nullptr;
    }

    inline event::InputEventReceiver* Window::getInputEventReceiver() const
    {
        ASSERT(m_receiver, "m_receiver is nullptr");
        return m_receiver;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
