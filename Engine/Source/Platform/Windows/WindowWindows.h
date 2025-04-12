#pragma once

#include "Common.h"
#include "Window.h"
#ifdef PLATFORM_WINDOWS

namespace v3d
{
namespace platform
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WindowWindows class. Windows platform
    */
    class V3D_API WindowWindows : public Window
    {
    public:

        explicit WindowWindows(const WindowParams& params, event::InputEventReceiver* receiver, Window* parent = nullptr) noexcept;
        ~WindowWindows();

        WindowWindows(const WindowWindows&) = delete;
        WindowWindows& operator=(const WindowWindows&) = delete;

        void minimize() override;
        void maximize() override;
        void restore() override;
        void show() override;
        void focus() override;

        void setFullScreen(bool value = true) override;
        void setResizeble(bool value = true) override;
        void setText(const std::wstring& text) override;
        void setSize(const math::Dimension2D& size) override;
        void setPosition(const math::Point2D& pos) override;

        bool isMaximized() const override;
        bool isMinimized() const override;
        bool isActive() const override;
        bool isFocused() const override;

        const math::Dimension2D& getSize() const override;
        const math::Point2D& getPosition() const override;

        NativeInstance getInstance() const override;
        NativeWindow getWindowHandle() const override;

        bool isValid() const override;

    private:

        bool initialize() override;
        bool update() override;
        void destroy() override;

        void fillKeyCodes();

        HINSTANCE               m_hInstance;
        HWND                    m_hWnd;
        std::wstring            m_className;

        u32                     m_timerID = 0;
        u64                     m_currnetTime = 0;
        u64                     m_lastMoveEvent = 0;
        u64                     m_lastSizeEvent = 0;

        HWND                    m_mouseHwnd = nullptr;
        u32                     m_mouseTrackedArea = 0;   // 0: not tracked, 1: client area, 2: non-client area

        Window*                 m_parent;
        std::vector<Window*>    m_children;

        LRESULT HandleInputMessage(UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT HandleSystemEvents(UINT message, WPARAM wParam, LPARAM lParam);


        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
#endif //PLATFORM_WINDOWS
