#pragma once

#include "Common.h"
#include "Platform/Window.h"

class MyApplication
{
public:

    MyApplication(int& argc, char** argv);
    virtual ~MyApplication();

    void    Initialize();
    bool    Running();
    int     Execute();

private:

    v3d::platform::Window* m_Window;
};
