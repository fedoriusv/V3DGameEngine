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
    class ForwardPointLightTest
    {
    public:

        explicit ForwardPointLightTest(renderer::CommandList& commandList) noexcept;
        ~ForwardPointLightTest();

        void Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights);

        void Draw(scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights);
        void Free();

    private:

        renderer::CommandList& m_CommandList;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;
    };


    class ForwardPointLightTextureTest
    {
    public:

        explicit ForwardPointLightTextureTest(renderer::CommandList& commandList) noexcept;
        ~ForwardPointLightTextureTest();

        void Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights);

        void Draw(scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights);
        void Free();

    private:

        renderer::CommandList& m_CommandList;

        utils::IntrusivePointer<renderer::GraphicsPipelineState> m_Pipeline;
        utils::IntrusivePointer<renderer::ShaderProgram> m_Program;

        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_Sampler;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureDiffuse;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureSpecular;
    };

} //namespace v3d