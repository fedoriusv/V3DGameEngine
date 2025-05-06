#pragma once

#include "Common.h"

#include "Renderer/Device.h"
#include "Renderer/RenderTargetState.h"
#include "Scene/CameraHandler.h"

namespace v3d
{
class Scene
{
public:

    struct DrawData
    {
        renderer::ShaderProgram* m_Program;
        renderer::GraphicsPipelineState* m_Pipeline;
        renderer::IndexBuffer* m_IdxBuffer;
        renderer::VertexBuffer* m_VtxBuffer;
    };

    void loadResources();

    static DrawData loadCube(renderer::Device* device, const renderer::RenderPassDesc& renderpassDesc);
    static void drawCube(renderer::Device* device, renderer::CmdListRender* cmdList, const DrawData& data, scene::CameraHandler* camera);

    static DrawData loadTriangle(renderer::Device* device, const renderer::RenderPassDesc& renderpassDesc);
    static void drawTriangle(renderer::Device* device, renderer::CmdListRender* cmdList, const DrawData& data, scene::CameraHandler* camera);

    void renderFrame();
};

} //namespace v3d