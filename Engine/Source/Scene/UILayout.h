#pragma once

#include "Common.h"

#include "Platform/Window.h"
#include "Events/InputEventHandler.h"
#include "Events/InputEventReceiver.h"

#include "Renderer/Device.h"
#include "Renderer/Texture.h"

struct ImGuiContext;

namespace v3d
{
namespace renderer
{
} // namespace render
namespace scene
{
    class UILayout
    {
    public:

        UILayout(renderer::Device* device);
        ~UILayout();

        void init(v3d::renderer::RenderTargetState* renderTarget);
        void render(renderer::CmdListRender* cmdList);
        void update(platform::Window* window, event::InputEventHandler* handler);

    private:

        void createResources(v3d::renderer::RenderTargetState* renderTarget);
        void destroyResources();

        renderer::Device* m_device;
        ImGuiContext*     m_IMGUIContext;

        renderer::Texture2D*    m_fontAtlas = nullptr;
        renderer::SamplerState* m_fontSampler;


        renderer::ShaderProgram* m_UIProgram;
        renderer::GraphicsPipelineState* m_UIPipeline;

        renderer::IndexBuffer*       m_UIIndexBuffer;
        renderer::VertexBuffer*      m_UIVertexBuffer;
        renderer::GeometryBufferDesc m_UIGeometryDesc;

        u32 m_offsetIB = 0;
        u32 m_offsetVB = 0;
    };

} //namespace scene
} //namespace v3d