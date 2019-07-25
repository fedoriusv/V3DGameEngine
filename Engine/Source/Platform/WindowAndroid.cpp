#include "WindowAndroid.h"
#include "Event/InputEventKeyboard.h"
#include "Event/InputEventMouse.h"
#include "Event/InputEventReceiver.h"
#include "Utils/Logger.h"

#ifdef PLATFORM_ANDROID
#include "AndroidNative.h"
#include "android_native_app_glue.h"

extern struct android_app* g_nativeAndroidApp;

namespace v3d
{
namespace platform
{

WindowAndroid::WindowAndroid(const WindowParam& params, event::InputEventReceiver* receiver)
    : Window(params, receiver)
    , m_sensorManager(NULL)
    , m_accelerometerSensor(NULL)
    , m_sensorEventQueue(NULL)
{
    LOG_DEBUG("WindowAndroid::WindowAndroid: Created Adroid window %llx", this);
    memset(&m_state, 0, sizeof(SavedState));
}

bool WindowAndroid::initialize()
{
    LOG_DEBUG("WindowAndroid::WindowAndroid::initialize");

    app_dummy(); //needs for link libV3D.a to *.so file
    ASSERT(g_nativeAndroidApp, "nullptr");

    g_nativeAndroidApp->userData = this;
	g_nativeAndroidApp->onAppCmd = WindowAndroid::handleCmdCallback;
	g_nativeAndroidApp->onInputEvent = WindowAndroid::handleInputCallback;

    // Prepare to monitor accelerometer
	m_sensorManager = ASensorManager_getInstance();
	m_accelerometerSensor = ASensorManager_getDefaultSensor(m_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	m_sensorEventQueue = ASensorManager_createEventQueue(m_sensorManager, g_nativeAndroidApp->looper, LOOPER_ID_USER, NULL, NULL);

	if (g_nativeAndroidApp->savedState != NULL) 
    {
		// We are starting with a previous saved state; restore from it.
		m_state = *reinterpret_cast<SavedState*>(g_nativeAndroidApp->savedState);
	}

    bool result = update();
    return result;
}

bool WindowAndroid::update()
{
    s32 ident;
    s32 events;
	struct android_poll_source* source;
    while((ident = ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0) 
    {
        // Process this event.      
		if (source != NULL) 
        {
		    source->process(g_nativeAndroidApp, source);
		}

        // If a sensor has data, process it now.
        if (ident == LOOPER_ID_USER)
        {
            if (m_accelerometerSensor != NULL)
            {
                ASensorEvent event;
                while (ASensorEventQueue_getEvents(m_sensorEventQueue, &event, 1) > 0)
                {
                    LOG_DEBUG("WindowAndroid::update accelerometer: x=%f y=%f z=%f",  event.acceleration.x, event.acceleration.y,  event.acceleration.z);
                }
            }
        }

        // Check if we are exiting.
        if (g_nativeAndroidApp->destroyRequested != 0)
        {
            return false;
        }
    }

    return true;
}

void WindowAndroid::destroy()
{
    LOG_DEBUG("WindowAndroid::destroy %llx", this);
}

WindowAndroid::~WindowAndroid()
{
    LOG_DEBUG("WindowAndroid::~WindowAndroid");
}

void WindowAndroid::minimize()
{
}

void WindowAndroid::maximize()
{
}

void WindowAndroid::restore()
{
}

void WindowAndroid::setFullScreen(bool value)
{
}

void WindowAndroid::setResizeble(bool value)
{
}

void WindowAndroid::setTextCaption(const std::string& text)
{
}

void WindowAndroid::setPosition(const core::Point2D& pos)
{
}

bool WindowAndroid::isMaximized() const
{
    return m_params._isMaximized;
}

bool WindowAndroid::isMinimized() const
{
    return  m_params._isMinimized;
}

bool WindowAndroid::isActive() const
{
    return false;
}

bool WindowAndroid::isFocused() const
{
    return false;
}

NativeInstance WindowAndroid::getInstance() const
{
    ASSERT(g_nativeAndroidApp, "nullptr");
    return nullptr;
    //return g_nativeAndroidApp->activity;
}

NativeWindows WindowAndroid::getWindowHandle() const
{
    ASSERT(g_nativeAndroidApp && g_nativeAndroidApp->window, "nullptr");
    return g_nativeAndroidApp->window;
}

void WindowAndroid::handleCmdCallback(struct android_app* app, int32_t cmd)
{
    LOG_DEBUG("WindowAndroid::WindowAndroid::handleCmdCallback");
	WindowAndroid* window = reinterpret_cast<WindowAndroid*>(app->userData);
	switch (cmd) 
    {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		g_nativeAndroidApp->savedState = malloc(sizeof(SavedState));
		*(reinterpret_cast<SavedState*>(g_nativeAndroidApp->savedState)) = window->m_state;
		g_nativeAndroidApp->savedStateSize = sizeof(SavedState);
		break;

	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (g_nativeAndroidApp->window != NULL) 
        {
            LOG_DEBUG("WindowAndroid::handleCmdCallback: APP_CMD_INIT_WINDOW");
			//engine_init_display(engine);
			//engine_draw_frame(engine);
		}
		break;

	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
        LOG_DEBUG("WindowAndroid::handleCmdCallback: APP_CMD_TERM_WINDOW");
		//engine_term_display(engine);
		break;

	case APP_CMD_GAINED_FOCUS:
        window->m_params._isFocused = true;
		// When our app gains focus, we start monitoring the accelerometer.
		if (window->m_accelerometerSensor != NULL) 
        {
			ASensorEventQueue_enableSensor(window->m_sensorEventQueue, window->m_accelerometerSensor);
			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(window->m_sensorEventQueue, window->m_accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
    
	case APP_CMD_LOST_FOCUS:
        window->m_params._isFocused = false;
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (window->m_accelerometerSensor != NULL) 
        {
			ASensorEventQueue_disableSensor(window->m_sensorEventQueue, window->m_accelerometerSensor);
		}
		// Also stop animating.
        LOG_DEBUG("WindowAndroid::handleCmdCallback: APP_CMD_LOST_FOCUS");
		//engine->animating = 0;
		//engine_draw_frame(engine);
		break;
	}
}

int32_t WindowAndroid::handleInputCallback(struct android_app* app, AInputEvent* event)
{
    LOG_DEBUG("WindowAndroid::WindowAndroid::handleInputCallback");
    WindowAndroid* window = reinterpret_cast<WindowAndroid*>(app->userData);
    //TODO
    switch(AInputEvent_getType(event))
    {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            f32 x = AMotionEvent_getX(event, 0);
            f32 y = AMotionEvent_getY(event, 0);
            LOG_DEBUG("WindowAndroid::handleInputCallback: event AINPUT_EVENT_TYPE_MOTION: x: %f, y: %f", x, y);
            return 1;
        }

        case AINPUT_EVENT_TYPE_KEY:
        {
            
        }

        default:
            LOG_DEBUG("WindowAndroid::handleInputCallback: event %u", AInputEvent_getType(event));
    }

	return 0;
}


} //namespace platform
} //namespace v3d
#endif //PLATFORM_ANDROID
