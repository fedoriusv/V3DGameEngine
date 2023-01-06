#pragma once

#include "Common.h"

#include "Platform/Window.h"
#include "Event/InputEventHandler.h"

#include "Renderer/Core/Context.h"
#include "Renderer/CommandList.h"

#include "Scene.h"

using namespace v3d;

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

    platform::Window* m_Window;
    event::InputEventHandler* m_InputEventHandler;

    renderer::Context* m_Context;
    renderer::CommandList* m_CommandList;

    Scene* m_Scene;
};
