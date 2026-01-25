#pragma once

#include "Common.h"
#include "Scene/Renderable.h"
#include "Scene/Component.h"
#include "Resource/Resource.h"
#include "Renderer/Buffer.h"
#include "Renderer/PipelineState.h"

namespace v3d
{
namespace resource
{
    class ModelResource;
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
    * @brief Mesh interface. Single geometry, Resource, Scene component
    */
    class Mesh : public Object, public resource::Resource, public ComponentBase<Mesh, Component>
    {
    public:

        enum class MeshType
        {
            Empty,
            Static,
            Skeletal
        };

        /**
        * @brief ShaderHeader struct.
        */
        struct MeshHeader : resource::ResourceHeader
        {
            MeshHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Mesh, toEnumType(MeshType::Empty))
            {
            }

            MeshHeader(MeshType type) noexcept
                : resource::ResourceHeader(resource::ResourceType::Mesh, toEnumType(type))
            {
            }
        };

        virtual ~Mesh();

        renderer::IndexBuffer* getIndexBuffer() const;
        renderer::VertexBuffer* getVertexBuffer(u32 stream) const;

        const renderer::VertexInputAttributeDesc& getVertexAttribDesc() const;
        renderer::PrimitiveTopology getTopology() const;

        const std::string_view getName() const;

        void setShadowsCast(bool value);
        bool isShadowsCasted() const;

    protected:

        using BaseType = Component;

        MeshHeader                           m_header;
        renderer::Device* const              m_device;
        renderer::VertexInputAttributeDesc   m_description;
        renderer::PrimitiveTopology          m_topology;
        renderer::IndexBuffer*               m_indexBuffer;
        std::vector<renderer::VertexBuffer*> m_vertexBuffer;
        math::AABB                           m_boundingBox;
        bool                                 m_castShadows;

        explicit Mesh(renderer::Device* device, MeshType type) noexcept;
        explicit Mesh(renderer::Device* device, const MeshHeader& header) noexcept;

        friend class MeshHelper;
    };

    inline renderer::IndexBuffer* Mesh::getIndexBuffer() const
    {
        return m_indexBuffer;
    }

    inline renderer::VertexBuffer* Mesh::getVertexBuffer(u32 stream) const
    {
        ASSERT(stream < m_vertexBuffer.size(), "range out");
        return m_vertexBuffer[stream];
    }

    inline const renderer::VertexInputAttributeDesc& Mesh::getVertexAttribDesc() const
    {
        return m_description;
    }

    inline renderer::PrimitiveTopology Mesh::getTopology() const
    {
        return m_topology;
    }

    inline const std::string_view Mesh::getName() const
    {
        return m_header.getName();
    }

    inline bool Mesh::isShadowsCasted() const
    {
        return m_castShadows;
    }

    inline void Mesh::setShadowsCast(bool value)
    {
        m_castShadows = value;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class MeshHelper
    {
    public:

        [[nodisard]] static Mesh* createStaticMesh(renderer::Device* device, renderer::CmdListRender* cmdList, const resource::ModelResource* resource, u32 model, u32 LOD, const std::string& name = "");

        [[nodisard]] static Mesh* createCube(renderer::Device* device, f32 extent = 1.0f, const std::string& name = "cube");
        [[nodisard]] static Mesh* createSphere(renderer::Device* device, f32 radius, u32 stacks = 64, u32 slices = 64, const std::string& name = "sphere");
        [[nodisard]] static Mesh* createCone(renderer::Device* device, f32 radius, f32 height, u32 segments = 64, const std::string& name = "cone");
        [[nodisard]] static Mesh* createCylinder(renderer::Device* device, f32 radius, f32 height, u32 segments = 64, const std::string& name = "cylinder");
        [[nodisard]] static Mesh* createPlane(renderer::Device* device, f32 width, f32 height, u32 segmentsX = 64, u32 segmentsY = 64, const std::string& name = "plane");
        [[nodisard]] static Mesh* createGrid(renderer::Device* device, f32 cellSize, u32 cellCountX = 64, u32 cellCountZ = 64, const std::string& name = "grid");
        [[nodisard]] static Mesh* createLineSegment(renderer::Device* device, const std::vector<math::float3>& points, const std::string& name = "line");
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::Mesh>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d