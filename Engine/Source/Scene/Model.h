#pragma once

#include "Common.h"
#include "Component.h"
#include "Resource/Resource.h"

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
        enum VertexProperies : u16
        {
            VertexProperies_Vertex  = 1 << 0,
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

        enum FaceProperties
        {
            FaceProperties_Triangle,
            FaceProperties_Point,
            FaceProperties_Line,
        };

        ModleHeader() {}
        ~ModleHeader() {};

        VertexProperies _content;
        FaceProperties  _face;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
