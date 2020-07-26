#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

#include "Platform/Window.h"
#include "Renderer/Context.h"
#include "Event/InputEventHandler.h"

#include "Scene/CameraArcballHelper.h"
#include "Scene/CameraFPSHelper.h"

#include "Utils/IntrusivePointer.h"
#include "Renderer/Object/RenderTargetState.h"
#include "Renderer/Object/PipelineState.h"
#include "Renderer/Object/ShaderProgram.h"

#include "Renderer/Object/SamplerState.h"
#include "Renderer/Object/Texture.h"

#include "Scene/ModelHelper.h"

#include "UnlitRender.h"
#include "DirectionalLightRender.h"
#include "PointLightRender.h"

class MyApplication
{
public:

    MyApplication(int& argc, char** argv);
    ~MyApplication();

    int Execute();

private:

    void Initialize();
    void Load();
    bool Running();
    void Update();
    void Exit();

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;

    v3d::renderer::Context* m_Context;
    v3d::renderer::CommandList* m_CommandList;

    v3d::scene::CameraArcballHelper* m_ArcballCameraHelper;
    v3d::scene::CameraFPSHelper* m_FPSCameraHelper;
    v3d::scene::CameraHelper* m_CameraHelper;

    v3d::utils::IntrusivePointer<v3d::renderer::RenderTargetState> m_RenderTarget;


    std::array<v3d::scene::ModelHelper*, 3> m_Geometry;

    v3d::core::Vector3D m_LightPosition = { 2.0, 1.0, -2.0 };
    v3d::core::Vector4D m_LightColor = { 1.0, 1.0, 1.0, 1.0 };
    using LightsProp = std::vector<std::tuple<v3d::core::Vector3D, v3d::core::Vector4D>>;
    LightsProp m_Lights;


    v3d::u32 m_DrawMode = 0;

    v3d::u32 m_GeometryIndex = 0;
    v3d::u32 m_LightIndex = 0;

    v3d::scene::Transform m_Transform;

    struct AxisDebug
    {
        v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_Pipeline;
        v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_Program;

        void Draw(v3d::renderer::CommandList* commandList, v3d::scene::Camera* camera);
        void Free();
    } m_AxisDebug;

    struct LightDebug
    {
        v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_Pipeline;
        v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_Program;

        void Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::Camera* camera, const LightsProp& lights);
        void Free();

    } m_LightDebug;


    v3d::UnlitTextureTest* m_UnLit;
    v3d::ForwardDirectionalLightTextureTest* m_Lambert;
    v3d::ForwardPointLightTest* m_PhongTextureless;

    struct Phong
    {
        v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_Pipeline;
        v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_Program;

        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_Sampler;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureDiffuse;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureSpecular;

        v3d::core::Vector4D m_LightPosition = { 110.0, 110.0, -110.0, 0.0 };

        void Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::core::Vector3D& lightPosition, const v3d::core::Vector4D& lightColor);
        void Free();

    } m_Phong;

    struct NormalMap
    {
        v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_Pipeline;
        v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_Program;

        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_SamplerColor;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureColor;

        v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_SamplerNormalmap;
        v3d::utils::IntrusivePointer<v3d::renderer::Texture2D> m_TextureNormalmap;

        v3d::core::Vector4D m_LightPosition = { 110.0, 110.0, -110.0, 0.0 };

        void Draw(v3d::renderer::CommandList* commandList, v3d::scene::ModelHelper* geometry, v3d::scene::CameraHelper* camera, const v3d::core::Vector3D& lightPosition, const v3d::core::Vector4D& lightColor);
        void Free();

    } m_Normalmap;
};
