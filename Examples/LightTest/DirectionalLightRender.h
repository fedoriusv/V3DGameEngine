#pragma once

#include "Utils/IntrusivePointer.h"
#include "Renderer/CommandList.h"

#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/Texture.h"
#include "Renderer/Object/SamplerState.h"

#include "Scene/ModelHelper.h"
#include "Scene/Camera.h"

namespace v3d
{
    class ForwardDirectionalLightTextureTest
    {
    public:

        explicit ForwardDirectionalLightTextureTest(renderer::CommandList& commandList) noexcept;
        ~ForwardDirectionalLightTextureTest();

        void Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights);

        void Draw(scene::ModelHelper* geometry, scene::Camera* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights);
        void Free();

    private:

        renderer::CommandList& m_CommandList;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;

        utils::IntrusivePointer<renderer::SamplerState> m_Sampler;
        utils::IntrusivePointer<renderer::Texture2D> m_Texture;
    };

} //namespace v3d