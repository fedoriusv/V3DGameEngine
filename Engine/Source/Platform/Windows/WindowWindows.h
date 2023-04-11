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
    class WindowWindows : public Window
    {
    public:

        explicit WindowWindows(const WindowParam& params, event::InputEventReceiver* receiver) noexcept;
        ~WindowWindows();

        WindowWindows(const WindowWindows&) = delete;
        WindowWindows& operator=(const WindowWindows&) = delete;

        void minimize() override;
        void maximize() override;
        void restore() override;

        void setFullScreen(bool value = true) override;
        void setResizeble(bool value = true) override;
        void setTextCaption(const std::string& text) override;
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
        const std::wstring  m_classname = L"V3DWin";

        LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
#endif //PLATFORM_WINDOWS
