#pragma once

#include "Utils/IntrusivePointer.h"
#include "Renderer/CommandList.h"

#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/Texture.h"
#include "Renderer/Object/SamplerState.h"

#include "Scene/ModelHelper.h"
#include "Scene/CameraHelper.h"

namespace v3d
{
    class UnlitTextureTest
    {
    public:

        explicit UnlitTextureTest(renderer::CommandList& commandList) noexcept;
        ~UnlitTextureTest();

        void Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc);

        void Draw(scene::ModelHelper* geometry, scene::CameraHelper* camera, const v3d::scene::Transform& transform);
        void Free();

    private:

        renderer::CommandList& m_CommandList;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;

        utils::IntrusivePointer<renderer::SamplerState> m_Sampler;
        utils::IntrusivePointer<renderer::Texture2D> m_Texture;
    };

} //namespace v3d