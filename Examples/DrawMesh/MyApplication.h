#pragma once

#include "Common.h"
#include "Platform/Window.h"
#include "Renderer/Core/Context.h"
#include "Event/InputEventHandler.h"

#include "Scene.h"

namespace app
{

class MyApplication
{
public:

    MyApplication(int& argc, char** argv) noexcept;
    ~MyApplication() = default;

    int Execute();

private:

    bool Initialize();
    bool Running();
    void Terminate();

    v3d::platform::Window*          m_Window;
    v3d::renderer::Context*         m_Context;
    v3d::renderer::CommandList*     m_CommandList;

    v3d::event::InputEventHandler*  m_InputEventHandler;

    Scene* m_Scene;
};

} //namespace app
