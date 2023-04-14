#pragma once

#include "Common.h"
#include "Event/InputEventHandler.h"

#include "Renderer/CommandList.h"
#include "Renderer/QueryRequest.h"

#include "Scene/CameraFPSHelper.h"
#include "Scene/ModelHelper.h"
#include "Scene/SceneProfiler.h"

#include "BaseDraw.h"

using namespace v3d;

class Scene final
{
public:

    Scene(renderer::CommandList& cmdList, const math::Dimension2D& size) noexcept;
    ~Scene();

    void Run(f32 dt);

    void mouseHandle(const event::InputEventHandler* handler, const event::MouseInputEvent* event);
    void touchHandle(const event::InputEventHandler* handler, const event::TouchInputEvent* event);
    void keyboardHandle(const event::InputEventHandler* handler, const event::KeyboardInputEvent* event);

private:

    void LoadScene();
    void UpdateScene();
    void FreeScene();

    renderer::CommandList& m_CommandList;

    scene::CameraFPSHelper* m_FPSCameraHelper;

    BaseDraw* m_BasePassDraw;
    BaseDraw::DrawLists m_DrawList;

    BaseDraw* m_SwapchainPassDraw;
    BaseDraw::DrawLists m_OffscreenDraw;

    std::vector<BaseDraw::RenderPolicy*> m_Renderers;
    std::vector<Object*> m_Resources;

    struct Measurements
    {
        void Print();

        struct QueryTiming
        {
            u64 _BeginTime = 0;
            u64 _EndTime = 0;
        };
        std::array<QueryTiming, 4> _QueryTimings;
        u32 _Draws = 0;
    };
    Measurements m_Measurements;

    scene::SceneProfiler m_SceneProfiler;
};
