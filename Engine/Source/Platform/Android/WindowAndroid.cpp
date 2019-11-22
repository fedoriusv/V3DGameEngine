#include "WindowAndroid.h"
#include "Event/InputEventTouch.h"
#include "Event/InputEventReceiver.h"
#include "Utils/Logger.h"

#include "AndroidNative.h"
#include "android_native_app_glue.h"

extern android_app* g_nativeAndroidApp;

namespace v3d
{
namespace platform
{

WindowAndroid::WindowAndroid(const WindowParam& params, event::InputEventReceiver* receiver)
    : Window(params, receiver)
    , m_sensorManager(NULL)
    , m_accelerometerSensor(NULL)
    , m_sensorEventQueue(NULL)
    , m_ready(false)
{
    LOG_DEBUG("WindowAndroid::WindowAndroid: Created Adroid window %llx", this);
    memset(&m_state, 0, sizeof(SavedState));

    //For Android always constant
    m_params._isFullscreen = true;
    m_params._isResizable = false;
    m_params._position = core::Point2D(0, 0);
    m_params._caption = L"Android App";
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

    bool result = true;
    while(!m_ready)
    {
        result = update();
        if (!result)
        {
            return false;
        }
    }


    fillKeyCodes();
    return true;
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
                    //LOG_DEBUG("WindowAndroid::update accelerometer: x=%f y=%f z=%f",  event.acceleration.x, event.acceleration.y,  event.acceleration.z);
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
    m_ready = false;
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
    LOG_DEBUG("WindowAndroid::setFullScreen not supported");
}

void WindowAndroid::setResizeble(bool value)
{
    LOG_DEBUG("WindowAndroid::setResizeble not supported");
}

void WindowAndroid::setTextCaption(const std::string& text)
{
    LOG_DEBUG("WindowAndroid::setTextCaption not supported");
}

void WindowAndroid::setPosition(const core::Point2D& pos)
{
    LOG_DEBUG("WindowAndroid::setPosition not supported");
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
    ASSERT(g_nativeAndroidApp && g_nativeAndroidApp->activity, "nullptr");
    return g_nativeAndroidApp->activity;
}

NativeWindows WindowAndroid::getWindowHandle() const
{
    ASSERT(g_nativeAndroidApp && g_nativeAndroidApp->window, "nullptr");
    return g_nativeAndroidApp->window;
}

bool WindowAndroid::isValid() const
{
    return m_ready && g_nativeAndroidApp->window != nullptr;
}

void WindowAndroid::handleCmdCallback(android_app* app, int32_t cmd)
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
        ASSERT(g_nativeAndroidApp->window, "nullptr");
        if (g_nativeAndroidApp->window != NULL &&  !window->m_ready) 
        {
            LOG_DEBUG("WindowAndroid::handleCmdCallback: APP_CMD_INIT_WINDOW");

            s32 width = ANativeWindow_getWidth(window->getWindowHandle());
            s32 height = ANativeWindow_getHeight(window->getWindowHandle());
            if (window->m_params._size.width != width || window->m_params._size.height != height)
            {
                LOG_WARNING("WindowAndroid::handleCmdCallback: different size: request: { %d, %d }, current { %d, %d }", window->m_params._size.width, window->m_params._size.height, width, height);
                window->m_params._size = core::Dimension2D(width,  height);
            }
            LOG_INFO("WindowAndroid::handleCmdCallback: window size: width %d, height %d", width, height);
            window->m_ready = true;
            window->notifyObservers();
        }
        break;

    case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        LOG_DEBUG("WindowAndroid::handleCmdCallback: APP_CMD_TERM_WINDOW");
        window->m_ready = false;
        window->notifyObservers();
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

int32_t WindowAndroid::handleInputCallback(android_app* app, AInputEvent* inputEvent)
{
    LOG_DEBUG("WindowAndroid::WindowAndroid::handleInputCallback");
    WindowAndroid* window = reinterpret_cast<WindowAndroid*>(app->userData);
    if (!window->m_receiver)
    {
        return 0;
    }

    switch(AInputEvent_getType(inputEvent))
    {
        case AINPUT_EVENT_TYPE_MOTION:
        {
            //s32 id = AMotionEvent_getPointerId(inputEvent, 0);
            //u32 countPointers = AMotionEvent_getPointerCount(inputEvent);
            s32 action = AMotionEvent_getAction(inputEvent);
            u32 pointers = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            f32 x = AMotionEvent_getX(inputEvent, 0);
            f32 y = AMotionEvent_getY(inputEvent, 0);
            //LOG_DEBUG("WindowAndroid::handleInputCallback: ID: %d event AINPUT_EVENT_TYPE_MOTION: x: %f, y: %f, motion flag %d, count %d, pointers %d", id, x, y, action, countPointers, pointers);

            auto getMotionAction = [](s32 action) -> event::TouchInputEvent::TouchMotionEvent
            {
                switch(action & AMOTION_EVENT_ACTION_MASK)
                {
                    case AMOTION_EVENT_ACTION_DOWN:
                        return event::TouchInputEvent::TouchMotionDown;
                    case AMOTION_EVENT_ACTION_UP:
                        return event::TouchInputEvent::TouchMotionUp;
                    case AMOTION_EVENT_ACTION_MOVE:
                        return event::TouchInputEvent::TouchMotionMove;
                    case AMOTION_EVENT_ACTION_POINTER_DOWN:
                        return event::TouchInputEvent::TouchMotionMultiTouchDown;
                    case AMOTION_EVENT_ACTION_POINTER_UP:
                        return event::TouchInputEvent::TouchMotionMultiTouchUp;
                    case AMOTION_EVENT_ACTION_SCROLL:
                        return event::TouchInputEvent::TouchMotionScroll;

                    default:
                        return event::TouchInputEvent::TouchMotionUnknown;
                };
            };

            event::TouchInputEvent* event = new(window->m_receiver->allocateInputEvent()) event::TouchInputEvent();
            event->_event = event::TouchInputEvent::TouchMotion;
            event->_motionEvent = getMotionAction(action);
            event->_position.x = static_cast<u32>(x);
            event->_position.y = static_cast<u32>(y);
            event->_pointers = pointers;

            event->_keyEvent = event::TouchInputEvent::TouchKeyPressUnknown;
            event->_key = event::KeyCode::KeyUknown;
            event->_modifers = event::KeyModifier_Uknown;

            window->m_receiver->pushEvent(event);
            return 1;
        }

        case AINPUT_EVENT_TYPE_KEY:
        {
            u32 code = AKeyEvent_getKeyCode(inputEvent);
            s32 action = AKeyEvent_getAction(inputEvent);
            s32 mods = AKeyEvent_getMetaState(inputEvent);
            LOG_DEBUG("WindowAndroid::handleInputCallback: event AINPUT_EVENT_TYPE_KEY: code: %d, action %d, mod %d", code, action, mods);

            auto getKeyEventAction = [](s32 action) -> event::TouchInputEvent::TouchKeyPressEvent
            {
                switch(action)
                {
                    case AKEY_EVENT_ACTION_DOWN:
                        return event::TouchInputEvent::TouchKeyPressDown;
                    case AKEY_EVENT_ACTION_UP:
                        return event::TouchInputEvent::TouchKeyPressUp;
                    case AKEY_EVENT_ACTION_MULTIPLE:
                        return event::TouchInputEvent::TouchKeyPressMultipress;

                    default:
                        return event::TouchInputEvent::TouchKeyPressUnknown;
                };
            };

            auto getKeyModsAction = [](s32 action) -> u8
            {
                switch(action)
                {
                    case AMETA_ALT_ON:
                    case AMETA_ALT_LEFT_ON:
                    case AMETA_ALT_RIGHT_ON:
                        return event::KeyModifier_Alt;
                    case AMETA_CTRL_ON:
                    case AMETA_CTRL_LEFT_ON:
                    case AMETA_CTRL_RIGHT_ON:
                        return event::KeyModifier_Ctrl;
                    case AMETA_SHIFT_ON:
                    case AMETA_SHIFT_LEFT_ON:
                    case AMETA_SHIFT_RIGHT_ON:
                        return event::KeyModifier_Shift;
                    case AMETA_META_ON:
                    case AMETA_META_LEFT_ON:
                    case AMETA_META_RIGHT_ON:
                        return event::KeyModifier_Meta;
                   case AMETA_CAPS_LOCK_ON:
                        return event::KeyModifier_CapsLock;

                    case AMETA_NONE:
                    default:
                        return event::KeyModifier_Uknown;
                };
            };

            event::TouchInputEvent* event = new(window->m_receiver->allocateInputEvent()) event::TouchInputEvent();
            event->_event = event::TouchInputEvent::TouchKey;
            event->_keyEvent = getKeyEventAction(action);
            event->_key = window->m_keyCodes.get(code);
            event->_modifers = getKeyModsAction(mods);
            
            event->_motionEvent = event::TouchInputEvent::TouchMotionUnknown;
            event->_position.x = 0;
            event->_position.y = 0;

            window->m_receiver->pushEvent(event);

            return 1;
        }

        default:
            LOG_DEBUG("WindowAndroid::handleInputCallback: event %u", AInputEvent_getType(inputEvent));
    }

	return 0;
}

void WindowAndroid::fillKeyCodes()
{
    m_keyCodes.add(event::KeyCode::KeyUknown, AKEYCODE_UNKNOWN);
    // m_keyCodes.add(event::KeyCode::KeyLButton, 0x01);
    // m_keyCodes.add(event::KeyCode::KeyRButton, 0x02);
    // m_keyCodes.add(event::KeyCode::KeyCancel, 0x03);
    // m_keyCodes.add(event::KeyCode::KeyMButton, 0x04);
    // m_keyCodes.add(event::KeyCode::KeyXButton1, 0x05);
    // m_keyCodes.add(event::KeyCode::KeyXButton2, 0x06);
    m_keyCodes.add(event::KeyCode::KeyBackspace, AKEYCODE_DEL);
    m_keyCodes.add(event::KeyCode::KeyTab, AKEYCODE_TAB);
    m_keyCodes.add(event::KeyCode::KeyClear, AKEYCODE_CLEAR);
    // m_keyCodes.add(event::KeyCode::KeyReturn, 0x0D);
    // m_keyCodes.add(event::KeyCode::KeyShift, 0x10);
    // m_keyCodes.add(event::KeyCode::KeyControl, 0x11);
    // m_keyCodes.add(event::KeyCode::KeyAlt, 0x12);
    // m_keyCodes.add(event::KeyCode::KeyPause, 0x13);
    // m_keyCodes.add(event::KeyCode::KeyCapital, 0x14);
    // m_keyCodes.add(event::KeyCode::KeyKana, 0x15);
    // m_keyCodes.add(event::KeyCode::KeyHanguel, 0x15);
    // m_keyCodes.add(event::KeyCode::KeyHangul, 0x15);
    // m_keyCodes.add(event::KeyCode::KeyJunja, 0x17);
    // m_keyCodes.add(event::KeyCode::KeyFinal, 0x18);
    // m_keyCodes.add(event::KeyCode::KeyHanja, 0x19);
    // m_keyCodes.add(event::KeyCode::KeyKanji, 0x19);
    m_keyCodes.add(event::KeyCode::KeyEscape, AKEYCODE_ESCAPE);
    // m_keyCodes.add(event::KeyCode::KeyConvert, 0x1C);
    // m_keyCodes.add(event::KeyCode::KeyNonconvert, 0x1D);
    // m_keyCodes.add(event::KeyCode::KeyAccept, 0x1E);
    // m_keyCodes.add(event::KeyCode::KeyModechange, 0x1F);
    // m_keyCodes.add(event::KeyCode::KeySpace, 0x20);
    // m_keyCodes.add(event::KeyCode::KeyPrior, 0x21);
    // m_keyCodes.add(event::KeyCode::KeyNext, 0x22);
    // m_keyCodes.add(event::KeyCode::KeyEnd, 0x23);
    m_keyCodes.add(event::KeyCode::KeyHome, AKEYCODE_HOME);
    // m_keyCodes.add(event::KeyCode::KeyLeft, 0x25);
    // m_keyCodes.add(event::KeyCode::KeyUp, 0x26);
    // m_keyCodes.add(event::KeyCode::KeyRight, 0x27);
    // m_keyCodes.add(event::KeyCode::KeyDown, 0x28);
    // m_keyCodes.add(event::KeyCode::KeySelect, 0x29);
    // m_keyCodes.add(event::KeyCode::KeyPrint, 0x2A);
    // m_keyCodes.add(event::KeyCode::KeyExecut, 0x2B);
    // m_keyCodes.add(event::KeyCode::KeySnapshot, 0x2C);
    m_keyCodes.add(event::KeyCode::KeyInsert, AKEYCODE_INSERT);
    m_keyCodes.add(event::KeyCode::KeyDelete, AKEYCODE_FORWARD_DEL);
    m_keyCodes.add(event::KeyCode::KeyHelp, AKEYCODE_HELP);
    m_keyCodes.add(event::KeyCode::KeyKey_0, AKEYCODE_0);
    m_keyCodes.add(event::KeyCode::KeyKey_1, AKEYCODE_1);
    m_keyCodes.add(event::KeyCode::KeyKey_2, AKEYCODE_2);
    m_keyCodes.add(event::KeyCode::KeyKey_3, AKEYCODE_3);
    m_keyCodes.add(event::KeyCode::KeyKey_4, AKEYCODE_4);
    m_keyCodes.add(event::KeyCode::KeyKey_5, AKEYCODE_5);
    m_keyCodes.add(event::KeyCode::KeyKey_6, AKEYCODE_6);
    m_keyCodes.add(event::KeyCode::KeyKey_7, AKEYCODE_7);
    m_keyCodes.add(event::KeyCode::KeyKey_8, AKEYCODE_8);
    m_keyCodes.add(event::KeyCode::KeyKey_9, AKEYCODE_9);
    m_keyCodes.add(event::KeyCode::KeyKey_A, AKEYCODE_A);
    m_keyCodes.add(event::KeyCode::KeyKey_B, AKEYCODE_B);
    m_keyCodes.add(event::KeyCode::KeyKey_C, AKEYCODE_C);
    m_keyCodes.add(event::KeyCode::KeyKey_D, AKEYCODE_D);
    m_keyCodes.add(event::KeyCode::KeyKey_E, AKEYCODE_E);
    m_keyCodes.add(event::KeyCode::KeyKey_F, AKEYCODE_F);
    m_keyCodes.add(event::KeyCode::KeyKey_G, AKEYCODE_G);
    m_keyCodes.add(event::KeyCode::KeyKey_H, AKEYCODE_H);
    m_keyCodes.add(event::KeyCode::KeyKey_I, AKEYCODE_I);
    m_keyCodes.add(event::KeyCode::KeyKey_J, AKEYCODE_J);
    m_keyCodes.add(event::KeyCode::KeyKey_K, AKEYCODE_K);
    m_keyCodes.add(event::KeyCode::KeyKey_L, AKEYCODE_L);
    m_keyCodes.add(event::KeyCode::KeyKey_M, AKEYCODE_M);
    m_keyCodes.add(event::KeyCode::KeyKey_N, AKEYCODE_N);
    m_keyCodes.add(event::KeyCode::KeyKey_O, AKEYCODE_O);
    m_keyCodes.add(event::KeyCode::KeyKey_P, AKEYCODE_P);
    m_keyCodes.add(event::KeyCode::KeyKey_Q, AKEYCODE_Q);
    m_keyCodes.add(event::KeyCode::KeyKey_R, AKEYCODE_R);
    m_keyCodes.add(event::KeyCode::KeyKey_S, AKEYCODE_S);
    m_keyCodes.add(event::KeyCode::KeyKey_T, AKEYCODE_T);
    m_keyCodes.add(event::KeyCode::KeyKey_U, AKEYCODE_U);
    m_keyCodes.add(event::KeyCode::KeyKey_V, AKEYCODE_V);
    m_keyCodes.add(event::KeyCode::KeyKey_W, AKEYCODE_W);
    m_keyCodes.add(event::KeyCode::KeyKey_X, AKEYCODE_X);
    m_keyCodes.add(event::KeyCode::KeyKey_Y, AKEYCODE_Y);
    m_keyCodes.add(event::KeyCode::KeyKey_Z, AKEYCODE_Z);
    // m_keyCodes.add(event::KeyCode::KeyLWin, 0x5B);
    // m_keyCodes.add(event::KeyCode::KeyRWin, 0x5C);
    // m_keyCodes.add(event::KeyCode::KeyApps, 0x5D);
    // m_keyCodes.add(event::KeyCode::KeySleep, 0x5F);
    m_keyCodes.add(event::KeyCode::KeyNumpad0, AKEYCODE_NUMPAD_0);
    m_keyCodes.add(event::KeyCode::KeyNumpad1, AKEYCODE_NUMPAD_1);
    m_keyCodes.add(event::KeyCode::KeyNumpad2, AKEYCODE_NUMPAD_2);
    m_keyCodes.add(event::KeyCode::KeyNumpad3, AKEYCODE_NUMPAD_3);
    m_keyCodes.add(event::KeyCode::KeyNumpad4, AKEYCODE_NUMPAD_4);
    m_keyCodes.add(event::KeyCode::KeyNumpad5, AKEYCODE_NUMPAD_5);
    m_keyCodes.add(event::KeyCode::KeyNumpad6, AKEYCODE_NUMPAD_6);
    m_keyCodes.add(event::KeyCode::KeyNumpad7, AKEYCODE_NUMPAD_7);
    m_keyCodes.add(event::KeyCode::KeyNumpad8, AKEYCODE_NUMPAD_8);
    m_keyCodes.add(event::KeyCode::KeyNumpad9, AKEYCODE_NUMPAD_9);
    // m_keyCodes.add(event::KeyCode::KeyMultiply, 0x6A);
    // m_keyCodes.add(event::KeyCode::KeyAdd, 0x6B);
    // m_keyCodes.add(event::KeyCode::KeySeparator, 0x6C);
    // m_keyCodes.add(event::KeyCode::KeySubtract, 0x6D);
    // m_keyCodes.add(event::KeyCode::KeyDecimal, 0x6E);
    // m_keyCodes.add(event::KeyCode::KeyDivide, 0x6F);
    m_keyCodes.add(event::KeyCode::KeyF1, AKEYCODE_F1);
    m_keyCodes.add(event::KeyCode::KeyF2, AKEYCODE_F2);
    m_keyCodes.add(event::KeyCode::KeyF3, AKEYCODE_F3);
    m_keyCodes.add(event::KeyCode::KeyF4, AKEYCODE_F4);
    m_keyCodes.add(event::KeyCode::KeyF5, AKEYCODE_F5);
    m_keyCodes.add(event::KeyCode::KeyF6, AKEYCODE_F6);
    m_keyCodes.add(event::KeyCode::KeyF7, AKEYCODE_F7);
    m_keyCodes.add(event::KeyCode::KeyF8, AKEYCODE_F8);
    m_keyCodes.add(event::KeyCode::KeyF9, AKEYCODE_F9);
    m_keyCodes.add(event::KeyCode::KeyF10, AKEYCODE_F10);
    m_keyCodes.add(event::KeyCode::KeyF11, AKEYCODE_F11);
    m_keyCodes.add(event::KeyCode::KeyF12, AKEYCODE_F12);
    // m_keyCodes.add(event::KeyCode::KeyF13, 0x7C);
    // m_keyCodes.add(event::KeyCode::KeyF14, 0x7D);
    // m_keyCodes.add(event::KeyCode::KeyF15, 0x7E);
    // m_keyCodes.add(event::KeyCode::KeyF16, 0x7F);
    // m_keyCodes.add(event::KeyCode::KeyF17, 0x80);
    // m_keyCodes.add(event::KeyCode::KeyF18, 0x81);
    // m_keyCodes.add(event::KeyCode::KeyF19, 0x82);
    // m_keyCodes.add(event::KeyCode::KeyF20, 0x83);
    // m_keyCodes.add(event::KeyCode::KeyF21, 0x84);
    // m_keyCodes.add(event::KeyCode::KeyF22, 0x85);
    // m_keyCodes.add(event::KeyCode::KeyF23, 0x86);
    // m_keyCodes.add(event::KeyCode::KeyF24, 0x87);
    // m_keyCodes.add(event::KeyCode::KeyNumlock, 0x90);
    // m_keyCodes.add(event::KeyCode::KeyScroll, 0x91);
    m_keyCodes.add(event::KeyCode::KeyLShift, AKEYCODE_SHIFT_LEFT);
    m_keyCodes.add(event::KeyCode::KeyRShift, AKEYCODE_SHIFT_RIGHT);
    m_keyCodes.add(event::KeyCode::KeyLControl, AKEYCODE_CTRL_LEFT);
    m_keyCodes.add(event::KeyCode::KeyRControl, AKEYCODE_CTRL_RIGHT);
    m_keyCodes.add(event::KeyCode::KeyLAlt, AKEYCODE_ALT_LEFT);
    m_keyCodes.add(event::KeyCode::KeyRAlt, AKEYCODE_ALT_RIGHT);
    // m_keyCodes.add(event::KeyCode::KeyPlus, 0xBB);
    // m_keyCodes.add(event::KeyCode::KeyComma, 0xBC);
    // m_keyCodes.add(event::KeyCode::KeyMinus, 0xBD);
    // m_keyCodes.add(event::KeyCode::KeyPeriod, 0xBE);
    // m_keyCodes.add(event::KeyCode::KeyAttn, 0xF6);
    // m_keyCodes.add(event::KeyCode::KeyCrSel, 0xF7);
    // m_keyCodes.add(event::KeyCode::KeyExSel, 0xF8);
    // m_keyCodes.add(event::KeyCode::KeyErEof, 0xF9);
    // m_keyCodes.add(event::KeyCode::KeyPlay, 0xFA);
    // m_keyCodes.add(event::KeyCode::KeyZoom, 0xFB);
    // m_keyCodes.add(event::KeyCode::KeyPA1, 0xFD);
    // m_keyCodes.add(event::KeyCode::KeyOem_Clear, 0xFE);
}


} //namespace platform
} //namespace v3d
