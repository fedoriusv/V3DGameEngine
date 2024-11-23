#pragma once

#include "Common.h"
#include "Scene/Component.h"
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
    class Mesh : public Object
    {
    public:

        Mesh() noexcept;
        ~Mesh();

        const renderer::VertexInputAttributeDesc& getVertexAttribDesc() const;

    public:

        renderer::VertexInputAttributeDesc      m_description;
        renderer::IndexBuffer*                  m_indexBuffer;
        std::vector<renderer::VertexBuffer*>    m_vertexBuffer;

        std::string                             m_name;
    };

    inline const renderer::VertexInputAttributeDesc& Mesh::getVertexAttribDesc() const
    {
        return m_description;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief StaticMesh class
    */
    class StaticMesh : public Mesh
    {
    public:

        StaticMesh() noexcept;
        ~StaticMesh();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class MeshHelper
    {
    public:

        [[nodisard]] static Mesh* createStaticMesh(renderer::Device* device, renderer::CmdListRender* cmdList, resource::MeshResource* resource);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d