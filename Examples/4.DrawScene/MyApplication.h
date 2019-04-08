#pragma once

#include "Common.h"
#include "Platform/Window.h"
#include "Renderer/CommandList.h"
#include "Renderer/Context.h"
#include "Event/InputEventHandler.h"

#include "Scene.h"
#include "Scene/CameraFPSHelper.h"

#include "Utils/Timer.h"

class MyApplication
{
public:

    MyApplication(int& argc, char** argv);
    ~MyApplication();

    int Execute();

private:

    void Initialize();
    void Update();
    bool Running(v3d::renderer::CommandList& cmd);
    void Exit();

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;

    v3d::renderer::Context* m_Context;
    v3d::renderer::CommandList* m_CommandList;

    v3d::scene::CameraFPSHelper* m_Camera;
    v3d::scene::Scene* m_Scene;


    bool m_captureMouseCursor;


    v3d::utils::Timer* m_Timer;
    v3d::f32 m_frameTime; // Sec/Frame
    const v3d::f32 m_timeFactor = 0.1f;
};
