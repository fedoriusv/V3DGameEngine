#pragma once

#include "Common.h"
#include "Scene/Scene.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;
    class RenderTechnique;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderPipelineStage
    {
    public:

        explicit RenderPipelineStage(RenderTechnique* technique, const std::string& id) noexcept;
        virtual ~RenderPipelineStage();

        virtual void create(Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;
        virtual void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;

        virtual void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;
        virtual void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame) = 0;

    protected:

        std::string m_id;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class RenderTechnique
    {
    public:

        void create(Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void destroy(Device* device, scene::SceneData& scene, scene::FrameData& frame);

        void prepare(Device* device, scene::SceneData& scene, scene::FrameData& frame);
        void execute(Device* device, scene::SceneData& scene, scene::FrameData& frame);

    protected:

        RenderTechnique() noexcept;
        virtual ~RenderTechnique();

        void addStage(const std::string& id, RenderPipelineStage* stage);

        struct Stage
        {
            std::string _id;
            RenderPipelineStage* _stage;
        };

        std::vector<Stage> m_stages;

        friend RenderPipelineStage;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct VertexFormatStandard
    {
        math::float3 position;
        math::float3 normal;
        math::float3 tangent;
        math::float3 binormal;
        math::float2 UV;
    };

    static renderer::VertexInputAttributeDesc VertexFormatStandardDesc(
        {
            renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, sizeof(VertexFormatStandard)),
        },
        {
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(VertexFormatStandard, position)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(VertexFormatStandard, normal)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(VertexFormatStandard, tangent)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(VertexFormatStandard, binormal)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, offsetof(VertexFormatStandard, UV)),
        }
    );

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d