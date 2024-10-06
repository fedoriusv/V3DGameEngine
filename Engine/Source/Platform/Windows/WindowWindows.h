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

        explicit WindowWindows(const WindowParams& params, event::InputEventReceiver* receiver, const Window* parent = nullptr) noexcept;
        ~WindowWindows();

        WindowWindows(const WindowWindows&) = delete;
        WindowWindows& operator=(const WindowWindows&) = delete;

        void minimize() override;
        void maximize() override;
        void restore() override;

        void setFullScreen(bool value = true) override;
        void setResizeble(bool value = true) override;
        void setTextCaption(const std::wstring& text) override;
        void setPosition(const math::Point2D& pos) override;

        bool isMaximized() const override;
        bool isMinimized() const override;
        bool isActive() const override;
        bool isFocused() const override;

        NativeInstance getInstance() const override;
        NativeWindows getWindowHandle() const override;

        bool isValid() const override;

    private:

        bool initialize() override;
        bool update() override;
        void destroy() override;

        void fillKeyCodes();

        HINSTANCE           m_hInstance;
        HWND                m_hWnd;

        u32                 m_timerID = 0;
        u64                 m_currnetTime = 0;
        u64                 m_lastMoveEvent = 0;
        u64                 m_lastSizeEvent = 0;

        const Window*       m_parent;

        LRESULT HandleInputMessage(UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT HandleSystemEvents(UINT message, WPARAM wParam, LPARAM lParam);


        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
#endif //PLATFORM_WINDOWS
