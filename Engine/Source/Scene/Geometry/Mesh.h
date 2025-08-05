#pragma once

#include "Common.h"
#include "Scene/Renderable.h"
#include "Resource/Resource.h"
#include "Renderer/Buffer.h"
#include "Renderer/PipelineState.h"

namespace v3d
{
namespace resource
{
    class MeshResource;
} //namespace resource
namespace renderer
{
    class Device;
    class CmdListRender;
} //namespace renderer
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Mesh class. Resource
    */
    class Mesh : public Object, public Renderable
    {
    public:

        const renderer::VertexInputAttributeDesc& getVertexAttribDesc() const;
        renderer::PrimitiveTopology getTopology() const;

    public:

        renderer::VertexInputAttributeDesc   m_description;
        renderer::PrimitiveTopology          m_topology;
        renderer::IndexBuffer*               m_indexBuffer;
        std::vector<renderer::VertexBuffer*> m_vertexBuffer;
        std::string                          m_name;

    protected:

        Mesh() noexcept;
        ~Mesh();
    };

    inline const renderer::VertexInputAttributeDesc& Mesh::getVertexAttribDesc() const
    {
        return m_description;
    }

    inline renderer::PrimitiveTopology Mesh::getTopology() const
    {
        return m_topology;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class MeshHelper
    {
    public:

        [[nodisard]] static Mesh* createStaticMesh(renderer::Device* device, renderer::CmdListRender* cmdList, resource::MeshResource* resource, const std::string& name = "");

        [[nodisard]] static Mesh* createCube(renderer::Device* device, renderer::CmdListRender* cmdList, f32 extent = 1.0f, const std::string& name = "cube");
        [[nodisard]] static Mesh* createSphere(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, u32 stacks = 64, u32 slices = 64, const std::string& name = "sphere");
        [[nodisard]] static Mesh* createCone(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, f32 height, u32 segments = 64, const std::string& name = "cone");
        [[nodisard]] static Mesh* createCylinder(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, f32 height, u32 segments = 64, const std::string& name = "cylinder");
        [[nodisard]] static Mesh* createPlane(renderer::Device* device, renderer::CmdListRender* cmdList, f32 width, f32 height, u32 segmentsX = 64, u32 segmentsY = 64, const std::string& name = "plane");
        [[nodisard]] static Mesh* createGrid(renderer::Device* device, renderer::CmdListRender* cmdList, f32 cellSize, u32 cellCountX = 64, u32 cellCountZ = 64, const std::string& name = "grid");
        [[nodisard]] static Mesh* createLine(renderer::Device* device, renderer::CmdListRender* cmdList, const std::vector<math::float3>& points, const std::string& name = "line");
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d