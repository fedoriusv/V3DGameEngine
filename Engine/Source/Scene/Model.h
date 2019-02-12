#pragma once

#include "Common.h"
#include "Component.h"
#include "Resource/Resource.h"
#include "Renderer/PipelineStateProperties.h"

namespace v3d
{
namespace scene
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ModleHeader meta info about Shader
    */
    struct ModleHeader : resource::ResourceHeader
    {
        enum VertexProperies : u32
        {
            VertexProperies_Position  = 1 << 0,
            VertexProperies_Normals = 1 << 1,
            VertexProperies_Tangent = 1 << 2,
            VertexProperies_TextCoord0 = 1 << 3,
            VertexProperies_TextCoord1 = 1 << 4,
            VertexProperies_TextCoord2 = 1 << 5,
            VertexProperies_TextCoord3 = 1 << 6,
            VertexProperies_Color0 = 1 << 7,
            VertexProperies_Color1 = 1 << 8,
            VertexProperies_Color2 = 1 << 9,
            VertexProperies_Color3 = 1 << 10,
        };

        typedef u16 VertexProperiesFlags;

        ModleHeader() noexcept;
        ~ModleHeader();

        struct MeshInfo
        {
            std::vector<u32>        _size;
            std::vector<u32>        _offset;
            std::vector<std::string> _names;
            u32                     _count;
            u32                     _globalSize;
        };

        MeshInfo                _vertex;
        MeshInfo                _index;

        renderer::PolygonMode   _mode;
        renderer::FrontFace     _frontFace;
        VertexProperiesFlags    _content;
        bool                    _localTransform;
        bool                    _indexBuffer;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxTextureCoordsIndex = 4;
    constexpr u32 k_maxVertexColorIndex = 4;

    /**
    * Model class
    */
    class Model : public Component, public resource::Resource
    {
    public:

        struct Vertex
        {
        };

        struct Index
        {
        };

        class Mesh final
        {
        public:

            Mesh() {};
            ~Mesh() {};

        private:

            std::vector<Vertex> m_vertices;
            std::vector<Index> m_indices;
        };

        explicit Model(ModleHeader* header) noexcept;
        ~Model();

        void init(stream::Stream* stream) override;
        bool load() override;

    private:

        std::vector<Mesh> m_meshes;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
