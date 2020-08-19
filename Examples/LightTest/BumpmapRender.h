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
    class ForwardNormalMapTest
    {
    public:

        explicit ForwardNormalMapTest(renderer::CommandList& commandList) noexcept;
        ~ForwardNormalMapTest();

        void Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights);

        void Draw(scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights);
        void Free();

    private:

        renderer::CommandList& m_CommandList;

        v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_Pipeline;
        v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_Program;

        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_SamplerColor;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureColor;

        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_SamplerNormalmap;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureNormalmap;
    };

    class ForwardParallaxMappingTest
    {
    public:

        explicit ForwardParallaxMappingTest(renderer::CommandList& commandList) noexcept;
        ~ForwardParallaxMappingTest();

        void Load(renderer::RenderTargetState* renderTarget, const renderer::VertexInputAttribDescription& desc, u32 countLights);

        void Draw(scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::scene::Transform& transform, const std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>& lights);
        void Free();

    private:

        renderer::CommandList& m_CommandList;

        v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_Pipeline;
        v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_Program;

        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_SamplerColor;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureColor;

        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_SamplerFilter;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureNormalmap;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureHeightmap;
    };

} //namespace v3d