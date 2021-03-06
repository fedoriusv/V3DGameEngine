#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

#include "Platform/Window.h"
#include "Renderer/Context.h"
#include "Event/InputEventHandler.h"

#include "SimpleTriangle.h"

class MyApplication
{
public:

    MyApplication(int& argc, char** argv);
    ~MyApplication();

    int Execute();

private:

    void Initialize();
    bool Running(v3d::renderer::CommandList& cmd);
    void Exit();

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;

    v3d::renderer::Context* m_Context;
    v3d::renderer::CommandList* m_CommandList;

    SimpleTriangle* m_Triangle;
};
