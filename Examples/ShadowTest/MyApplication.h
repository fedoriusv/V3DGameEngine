#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"

#include "Platform/Window.h"
#include "Event/InputEventHandler.h"

#include "Scene/CameraFPSHelper.h"
#include "Scene/ModelHelper.h"

#include "Renderer/Core/Context.h"
#include "Renderer/CommandList.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/SamplerState.h"
#include "Renderer/Texture.h"

#include "ShadowMap.h"
#include "Debug.h"

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
    void Update(v3d::f32 dt);
    void Exit();

    v3d::platform::Window* m_Window;
    v3d::event::InputEventHandler* m_InputEventHandler;

    v3d::renderer::Context* m_Context;
    v3d::renderer::CommandList* m_CommandList;

    v3d::scene::CameraFPSHelper* m_FPSCameraHelper;
    v3d::scene::ModelHelper* m_Scene;

    v3d::utils::IntrusivePointer<v3d::renderer::SamplerState> m_ShadowSampler;

    v3d::utils::IntrusivePointer<v3d::renderer::RenderTargetState> m_RenderTarget;
    v3d::scene::Transform m_Transform;

    void DrawDirectionLightMode(bool enablePCF, bool cascaded);
    void DrawPointLightMode(bool enablePCF);

    v3d::core::Vector3D m_SunDirection = { 5.0f, 5.0f, 5.0f };

    v3d::ShadowMapping* m_ShadowMapping;
    v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_ShadowMappingPipeline;
    v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_ShadowMappingProgram;

    v3d::CascadedShadowMapping* m_CascadeShadowMapping;
    v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_CascadeShadowMappingPipeline;
    v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_CascadeShadowMappingProgram;

    v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_CascadeShadowMappingPipelineDebug;
    v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_CascadeShadowMappingProgramDebug;


    v3d::ShadowMappingPoint* m_ShadowMappingPoint;
    v3d::utils::IntrusivePointer<v3d::renderer::GraphicsPipelineState> m_ShadowMappingPointPipeline;
    v3d::utils::IntrusivePointer<v3d::renderer::ShaderProgram> m_ShadowMappingPointProgram;

    enum ShadowMode
    {
        DirectionLight = 0,
        DirectionLightCascadeShadowing,
        PointLight,

        MaxMode,
        FirstMode = DirectionLight

    };
    ShadowMode m_Mode;
    bool m_PercentageCloserFiltering;

    v3d::LightDebug m_LightDebug;
    bool m_Debug;
};
