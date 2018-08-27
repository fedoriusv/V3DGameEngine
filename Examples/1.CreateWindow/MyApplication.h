#pragma once

#include "Common.h"
#include "Platform/Window.h"
#include "Event/InputEventReceiver.h"

class MyApplication
{
public:

    MyApplication(int& argc, char** argv);
    virtual ~MyApplication();

    void    Initialize();
    bool    Running();
    int     Execute();
    void    Exit();

private:

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;

};
