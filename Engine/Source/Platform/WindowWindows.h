#pragma once

#include "Window.h"
#include <windows.h>

namespace v3d
{
namespace platform
{

    class WindowWindows : public Window
    {
    public:

        explicit WindowWindows(const WindowParam& params);
        ~WindowWindows();

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

    private:

        bool initialize() override;
        bool update() override;
        void destroy() override;

        HINSTANCE m_hInstance;
        HWND m_hWnd;
        const std::wstring m_classname = L"V3DWin";

        LRESULT HandleEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    };

} //namespace platform
} //namespace v3d
