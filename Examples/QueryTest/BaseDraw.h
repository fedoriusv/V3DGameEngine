#pragma once

#include "Common.h"

#include "Renderer/CommandList.h"
#include "Renderer/SamplerState.h"
#include "Renderer/Texture.h"
#include "Renderer/RenderTargetState.h"
#include "Renderer/StreamBuffer.h"
#include "Renderer/QueryRequest.h"

#include "Scene/SceneProfiler.h"

using namespace v3d;

class BaseDraw
{
public:

    struct ProgramParams
    {
        struct UBO
        {
            core::Matrix4D projectionMatrix;
            core::Matrix4D viewMatrix;
            core::Matrix4D modelMatrix;
            core::Matrix4D normalMatrix;
        };

        UBO _ConstantBuffer;
        renderer::SamplerState* _Sampler;
        renderer::Texture2D* _Texture;
    };

    struct MeshInfo
    {
        renderer::StreamBufferDescription _BufferDescription;
        renderer::VertexInputAttribDescription _VertexLayoutDescription;
        renderer::DrawProperties _DrawProperties;
    };

    class RenderPolicy
    {
    public:

        RenderPolicy() noexcept = default;
        virtual ~RenderPolicy() = default;

        virtual void Init(renderer::CommandList& commandList, const renderer::RenderTargetState* renderTaget) = 0;
        virtual void Render(renderer::CommandList& commandList, const ProgramParams& params, const MeshInfo* meshData) = 0;
    };

    struct DrawLists
    {
        std::vector<std::tuple<ProgramParams*, MeshInfo*>> _DrawState;
        RenderPolicy* _Render;

        renderer::QueryTimestampRequest* _TimeStampQuery;
        scene::SceneProfiler* _Profiler;
        u32 _DrawedLastFrame;
    };

    BaseDraw() noexcept = default;
    virtual ~BaseDraw() = default;

    virtual const renderer::RenderTargetState* GetRenderTarget() const = 0;

    virtual void Init(renderer::CommandList& cmdList, const core::Dimension2D& size) = 0;
    virtual void Draw(renderer::CommandList& cmdList, DrawLists& drawList) = 0;
};