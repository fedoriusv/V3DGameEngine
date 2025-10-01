#pragma once

#include "Common.h"
#include "Renderer/PipelineState.h"

namespace v3d
{
namespace scene
{

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct VertexFormatSimpleLit
    {
        math::float3 position;
        math::float3 normal;
        math::float2 UV;
    };

    static renderer::VertexInputAttributeDesc VertexFormatSimpleLitDesc(
        {
            renderer::VertexInputAttributeDesc::InputBinding(0,  renderer::InputRate::InputRate_Vertex, sizeof(VertexFormatSimpleLit)),
        },
        {
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(VertexFormatSimpleLit, position)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32B32_SFloat, offsetof(VertexFormatSimpleLit, normal)),
            renderer::VertexInputAttributeDesc::InputAttribute(0, 0, renderer::Format_R32G32_SFloat, offsetof(VertexFormatSimpleLit, UV)),
        }
    );

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d
