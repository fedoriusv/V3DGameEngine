#pragma once

#include "Window.h"
#ifdef PLATFORM_ANDROID
#include "AndroidNative.h"

namespace v3d
{
namespace platform
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class WindowAndroid : public Window
    {
    public:

        WindowAndroid(const WindowParam& params, event::InputEventReceiver* receiver);
        ~WindowAndroid();

        WindowAndroid(const WindowAndroid&) = delete;
        WindowAndroid& operator=(const WindowAndroid&) = delete;

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

    private:

        bool initialize() override;
        bool update() override;
        void destroy() override;

        struct SavedState 
        {
	        f32 _angle;
	        u32 _x;
	        u32 _y;
        };

        ASensorManager*     m_sensorManager;
	    const ASensor*      m_accelerometerSensor;
	    ASensorEventQueue*  m_sensorEventQueue;
        SavedState          m_state;
        bool                m_initialized;

        static void handleCmdCallback(struct android_app* app, int32_t cmd);
        static int32_t handleInputCallback(struct android_app* app, AInputEvent* event);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace platform
} //namespace v3d
#endif //PLATFORM_ANDROID
