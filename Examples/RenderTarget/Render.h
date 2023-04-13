#pragma once

#include "Common.h"

#include "Renderer/Core/Context.h"
#include "Renderer/CommandList.h"

#include "Renderer/PipelineState.h"
#include "Renderer/ShaderProgram.h"

#include "Scene/CameraArcballHelper.h"

class DrawPolicy
{
public:

    DrawPolicy() noexcept = default;
    virtual ~DrawPolicy() = default;

    virtual void Init(v3d::renderer::CommandList* commandList, v3d::renderer::RenderTargetState* target) = 0;
    virtual void Bind(v3d::renderer::CommandList* commandList) = 0;
    virtual void Draw(v3d::renderer::CommandList* commandList) = 0;
};

class BaseRender
{
public:

    BaseRender() = default;
    virtual ~BaseRender() = default;

    virtual void Init(v3d::renderer::CommandList* commandList) = 0;
    virtual void Render(v3d::renderer::CommandList* commandList, DrawPolicy* draw = nullptr) = 0;
};

class SceneRenderer final
{
public:

    explicit SceneRenderer(v3d::renderer::CommandList& m_CommandList) noexcept;
    ~SceneRenderer();

    void Prepare(const v3d::math::Dimension2D& size);
    void Render(v3d::f32 dt);

    v3d::scene::CameraArcballHelper* getActiveCamera() const;

private:

    v3d::renderer::CommandList& m_CommandList;

    v3d::scene::CameraArcballHelper* m_Camera;

    BaseRender* m_BasePass;
    BaseRender* m_DownsamplePass;
    BaseRender* m_OffcreenPass;

    DrawPolicy* m_Draws;
};



