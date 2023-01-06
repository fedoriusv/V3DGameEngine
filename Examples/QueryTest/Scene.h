#pragma once

#include "Renderer/CommandList.h"
#include "Event/InputEventHandler.h"

#include "Scene/CameraFPSHelper.h"
#include "Scene/ModelHelper.h"

using namespace v3d;

class Scene final
{
public:

    Scene(renderer::CommandList& cmdList, const core::Dimension2D& size) noexcept;
    ~Scene();

    void Run(f32 dt);

    void mouseHandle(const event::InputEventHandler* handler, const event::MouseInputEvent* event);
    void touchHandle(const event::InputEventHandler* handler, const event::TouchInputEvent* event);
    void keyboardHandle(const event::InputEventHandler* handler, const event::KeyboardInputEvent* event);

private:

    class BaseRender
    {
    public:

        BaseRender() = default;
        virtual ~BaseRender() = default;

        virtual void Init(renderer::CommandList& commandList, const renderer::VertexInputAttribDescription& desc, const renderer::RenderTargetState* renderTaget) = 0;
        virtual void Render(renderer::CommandList& commandList) = 0;
        virtual void Destroy(renderer::CommandList& commandList) = 0;
    };

    renderer::CommandList& m_CommandList;

    scene::CameraFPSHelper* m_FPSCameraHelper;

    renderer::RenderTargetState* m_RenderTarget;
    renderer::Texture2D* m_ColorAttachment;
    renderer::Texture2D* m_DepthAttachment;

    BaseRender* m_TexturedRender;
};