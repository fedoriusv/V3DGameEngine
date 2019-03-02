#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"
#include "Renderer/BufferProperties.h"

#include "Scene/Transform.h"

namespace v3d
{
namespace renderer
{
    class StreamBuffer;
} //namespace renderer

namespace scene
{
    class Model;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ModelHelper
    */
    class ModelHelper final
    {
    public:

        ModelHelper() = delete;
        ModelHelper(const ModelHelper&) = delete;

        explicit ModelHelper(renderer::CommandList& cmdList, const std::vector<const Model*>& models) noexcept;
        ~ModelHelper();

        void drawModel(s32 index = -1);

        const renderer::VertexInputAttribDescription& getVertexInputAttribDescription(u32 modelIndex, u32 meshIndex) const;
        Transform& getTransform();

        static ModelHelper* createModelHelper(renderer::CommandList& cmdList, const std::vector<const Model*>& models);

    private:

        struct DrawProps
        {
            u32 _start;
            u32 _count;
            u32 _startInstance;
            u32 _countInstance;

            bool _indexDraws;
        };

        renderer::CommandList& m_cmdList;

        std::vector<const Model*> m_models;
        std::vector<std::tuple<renderer::StreamBufferDescription, DrawProps>> m_drawState;

        Transform m_tramsform;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
