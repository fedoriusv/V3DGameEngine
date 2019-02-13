#pragma once

#include "Common.h"
#include "Component.h"
#include "Resource/Resource.h"
#include "Renderer/PipelineStateProperties.h"
#include "Renderer/BufferProperties.h"

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
            struct Data
            {
                u32 _size;
                u32 _offset;
                u32 _count;
            };

            std::vector<Data>        _data;
            u32                      _countElements;
            u32                      _globalSize;
        };

        MeshInfo                _vertex;
        MeshInfo                _index;

        renderer::PolygonMode   _mode;
        renderer::FrontFace     _frontFace;
        VertexProperiesFlags    _content;
        bool                    _localTransform;
        bool                    _indexBuffer;

        std::vector<std::string> _names;
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

        class Mesh final
        {
        public:

            Mesh() noexcept;
            ~Mesh();

            u8* getVertexData() const;
            u8* getIndexData() const;

            const renderer::VertexInputAttribDescription& getVertexInputAttribDesc() const;

        private:

            friend Model;

            void fillVertexData(u32 count, u8* data, u32 size);
            void fillIndexData(u32 count, u8* data, u32 size);

            struct BufferData
            {
                u8* _data;
                u32 _size;
            };

            BufferData m_vertexData;
            BufferData m_indexData;

            u32 m_vertexCount;
            u32 m_indexCount;

            renderer::VertexInputAttribDescription m_description;
        };

        explicit Model(ModleHeader* header) noexcept;
        ~Model();

        const ModleHeader& getModleHeader() const;

        Model::Mesh* getMeshByIndex(u32 index);

        void init(stream::Stream* stream) override;
        bool load() override;

    private:

        friend class ModelHelper;

        stream::Stream* m_vertexModelData;
        stream::Stream* m_indexModelData;

        std::vector<Mesh*> m_meshes;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
