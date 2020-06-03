#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"

#include "Renderer/Context.h"
#include "Renderer/CommandList.h"
#include "Scene/CameraArcballHelper.h"

#include "Event/InputEventMouse.h"
#include "Event/InputEventTouch.h"
#include "Event/InputEventHandler.h"

#include "Renderer/Object/Texture.h"
#include "Renderer/Object/SamplerState.h"
#include "Scene/ModelHelper.h"

#include "TextureRender.h"

namespace app
{

class Scene
{
public:

    Scene(v3d::renderer::Context* context) noexcept;
    ~Scene();

    bool Run();
    void SendExitSignal();

    static void MouseCallback(Scene* scene, v3d::event::InputEventHandler* handler, const v3d::event::MouseInputEvent* event);
    static void TouchCallback(Scene* scene, v3d::event::InputEventHandler* handler, const v3d::event::TouchInputEvent* event);

private:

    v3d::renderer::Context*     m_Context;
    v3d::renderer::CommandList* m_CommandList;

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
    void Update();
    void Draw();
    void Exit();

    v3d::scene::CameraArcballHelper* m_Camera;
    v3d::TextureRender*              m_Render;

    struct Model
    {
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D>    m_Texture;
        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_Sampler;
        v3d::scene::ModelHelper* m_Model;
    };
    Model m_Voyager;

    void LoadVoyager();
};

} //namespace app