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

private:

    void Initialize();
    bool Running();
    void Exit();

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;
};
