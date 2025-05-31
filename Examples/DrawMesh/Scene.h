#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
#include "Renderer/Render.h"
#include "Renderer/Device.h"
#include "Renderer/Texture.h"
#include "Renderer/SamplerState.h"
#include "Scene/Model.h"
#include "Events/Input/InputEventMouse.h"
#include "Events/Input/InputEventTouch.h"
#include "Events/Input/InputEventHandler.h"
#include "TextureRender.h"
#include "Scene/Camera/CameraArcballHandler.h"

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

private:

    v3d::renderer::Device*        m_Device;
    v3d::renderer::Swapchain*     m_Swapchain;
    v3d::renderer::CmdListRender* m_CmdList;

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

    v3d::scene::CameraArcballHandler* m_Camera;
    app::TextureRender*               m_Render;

    struct Model
    {
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D>    m_Texture;
        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_Sampler;
        v3d::renderer::VertexInputAttributeDesc                   m_InputAttrib;
        std::vector<std::tuple<v3d::renderer::GeometryBufferDesc, app::DrawProperties>> m_Props;
        v3d::scene::Model* m_Model;
    };

    std::list<Model*> m_Models;

    void LoadVoyager();
};

} //namespace app