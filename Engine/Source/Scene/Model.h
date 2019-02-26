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
    struct ModelHeader : resource::ResourceHeader
    {
        enum ModelContent : u32
        {
            ModelContext_Empty = 0,
            ModelContext_Mesh = 1 << 0,
            ModelContext_Material = 1 << 1
        };
        typedef u32 ModelContentFlags;

        enum VertexProperies : u32
        {
            VertexProperies_Empty = 0,
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

        ModelHeader() noexcept;
        ~ModelHeader();

        struct MeshInfo
        {
            struct Data
            {
                u64 _size;
                u64 _offset;
                u64 _count;
            };

            std::vector<Data>        _data;
            u64                      _countElements;
            u64                      _globalSize;
            std::vector<std::string> _names;
            bool                     _present;
        };

        struct MaterialInfo
        {
            u64                      _countElements;
            u64                      _globalSize;
            std::vector<std::string> _names;
            bool                     _present;
        };

        ModelContentFlags       _modelContent;

        renderer::PolygonMode   _mode;
        renderer::FrontFace     _frontFace;
        VertexProperiesFlags    _vertexContent;
        bool                    _localTransform;

        MeshInfo                _vertex;
        MeshInfo                _index;
        MaterialInfo            _materials;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxTextureCoordsIndex = 4;
    constexpr u32 k_maxVertexColorIndex = 4;

    /**
    * Model class. Component, Resource
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

        explicit Model(ModelHeader* header) noexcept;
        ~Model();

        const ModelHeader& getModelHeader() const;

        Model::Mesh* getMeshByIndex(u32 index) const;

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
