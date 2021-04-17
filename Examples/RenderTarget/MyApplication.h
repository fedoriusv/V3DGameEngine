#pragma once

#include "Common.h"

#include "Platform/Window.h"
#include "Event/InputEventHandler.h"

#include "Renderer/Context.h"
#include "Renderer/CommandList.h"

#include "Render.h"

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

    v3d::renderer::Context* m_Context;
    v3d::renderer::CommandList* m_CommandList;

    SceneRenderer* m_Scene;
};
