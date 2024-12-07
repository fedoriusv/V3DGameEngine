#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
#include "Renderer/Render.h"
#include "Renderer/Device.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"
#include "Scene/CameraFPSHandler.h"
#include "Scene/Model.h"
#include "Events/InputEventMouse.h"
#include "Events/InputEventTouch.h"
#include "Events/InputEventHandler.h"
#include "Task/TaskScheduler.h"
#include "Render.h"

namespace app
{

class Scene
{
public:

    Scene(v3d::renderer::Device* device, v3d::renderer::Swapchain* swapchain) noexcept;
    ~Scene();

    void Run(v3d::f32 dt);
    void SendExitSignal();

    static void MouseCallback(Scene* scene, v3d::event::InputEventHandler* handler, const v3d::event::MouseInputEvent* event);
    static void TouchCallback(Scene* scene, v3d::event::InputEventHandler* handler, const v3d::event::TouchInputEvent* event);

public:

    v3d::renderer::Device*        m_Device;
    v3d::renderer::Swapchain*     m_Swapchain;
    v3d::task::TaskScheduler      m_Worker;

private:

    enum class States
    {
        StateInit,
        StateLoad,
        StateDraw,
        StateExit
    };

    States m_CurrentState;

    void Init();
    void Load();
    void Draw(v3d::f32 dt);
    void Exit();

    v3d::scene::CameraFPSHandler* m_Camera;
    app::Render*                  m_Render;

    v3d::renderer::RenderTargetState* m_RenderTarget;

    struct Model
    {
        //TODO material
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D>                          m_Texture;
        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState>                       m_Sampler;

        v3d::renderer::GraphicsPipelineState*                                           m_Pipeline;
        v3d::renderer::VertexInputAttributeDesc                                         m_InputAttrib;
        std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, app::DrawProperties>> m_Props;
        v3d::scene::Model*                                                              m_Model;

    };
    std::list<Model*> m_Models;

    struct ModelsGroup
    {
        std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, app::DrawProperties>> m_InputProps;
        v3d::renderer::GraphicsPipelineState*                                           m_Pipeline;
    };

    std::vector<ModelsGroup*> m_ModelGroups;
    std::vector<std::tuple<TextureRenderWorker*, v3d::task::Task*, v3d::u32>> m_RenderGroups;
};

} //namespace app