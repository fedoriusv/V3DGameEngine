#pragma once

#include "Common.h"
#include "Platform/Window.h"
#include "Event/InputEventReceiver.h"


class MyApplication
{
public:

    MyApplication(int& argc, char** argv);
    ~MyApplication();

    int Execute();

    void OnCreate(v3d::platform::Window* window);
    void OnDestroy(v3d::platform::Window* window);
    void OnResize(v3d::platform::Window* window);

private:

    void Initialize();
    bool Running();
    void Exit();

    v3d::platform::Window* m_MainWindow;
    v3d::platform::Window* m_UIWindow;
    v3d::event::InputEventReceiver* m_InputReceiver;

    v3d::event::InputEventHandler* m_InputEventHandler;

public:

    bool m_teminateApp = false;
};
