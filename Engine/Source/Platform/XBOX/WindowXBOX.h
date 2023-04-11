#pragma once

#include "Common.h"
#include "Window.h"
#ifdef PLATFORM_XBOX

struct _APPSTATE_REGISTRATION;

namespace v3d
{
namespace platform
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief WindowWindows class. XBOX platform
    */
    class WindowXBOX : public Window
    {
    public:

        WindowXBOX(const WindowParam& params, event::InputEventReceiver* receiver);
        ~WindowXBOX();

        WindowXBOX(const WindowXBOX&) = delete;
        WindowXBOX& operator=(const WindowXBOX&) = delete;

        void minimize() override;
        void maximize() override;
        void restore() override;

        void setFullScreen(bool value = true) override;
        void setResizeble(bool value = true) override;
        void setTextCaption(const std::string& text) override;
        void setPosition(const core::Point2D& pos) override;

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

        HINSTANCE           m_hInstance;
        HWND                m_hWnd;
        const std::wstring  m_classname = L"V3DXBOX";

        LRESULT ReceiverMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        _APPSTATE_REGISTRATION* m_hPLM;
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    public:

        static HANDLE s_suspendComplete;
        static HANDLE s_signalResume;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
#endif //PLATFORM_XBOX
