#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
#include "Renderer/Render.h"
#include "Renderer/Device.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"
#include "Scene/Camera/CameraFreeFlyHandler.h"
#include "Scene/ModelHandler.h"
#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventTouch.h"
#include "Events/Input/InputEventHandler.h"
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
    static void KeyboardCallback(Scene* scene, v3d::event::InputEventHandler* handler, const v3d::event::KeyboardInputEvent* event);

public:

    v3d::renderer::Device*                  m_Device;
    v3d::renderer::Swapchain*               m_Swapchain;
    v3d::renderer::CmdListRender*           m_CmdList;
    v3d::renderer::SyncPoint*               m_Sync;

    v3d::task::TaskScheduler                m_Worker;

    v3d::renderer::ShaderProgram*           m_ProgramBackbuffer;
    v3d::renderer::GraphicsPipelineState*   m_PipelineBackbuffer;
    v3d::renderer::RenderTargetState*       m_renderTargetBackbuffer;
    v3d::renderer::SamplerState*            m_samplerBackbuffer;

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

    v3d::scene::CameraFreeFlyHandler*   m_Camera;
    app::Render*                        m_Render;

    v3d::renderer::RenderTargetState*   m_RenderTarget;

    struct ParameterData
    {
        v3d::renderer::Texture2D*      _Texture;
        v3d::renderer::SamplerState*   _Sampler;
        v3d::math::Matrix4D            _Transform;
    };

    struct ModelData
    {
        //TODO material

        v3d::renderer::GraphicsPipelineState*                                           _Pipeline;
        v3d::renderer::VertexInputAttributeDesc                                         _InputAttrib;
        std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, app::DrawProperties>> _Props;
        v3d::scene::Model*                                                              _Model;

    };
    std::set<ModelData*>                                m_Models;
    std::vector<v3d::renderer::GraphicsPipelineState*>  m_Pipelines;

    struct ModelsGroup
    {
        std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, app::DrawProperties>> _InputProps;
        ParameterData                                                                   _Parameters;
        v3d::u32                                                                        _PipelineID;
};

    std::vector<ModelsGroup> m_ModelInstances; //sorted by material
    std::vector<std::tuple<TextureRenderWorker*, v3d::task::Task*, v3d::u32, v3d::u32, v3d::u32>> m_RenderGroups;
};

} //namespace app