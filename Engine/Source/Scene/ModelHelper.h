#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
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

        void draw(s32 index = -1);
        u32 getDrawStatesCount() const;

        const renderer::VertexInputAttribDescription& getVertexInputAttribDescription(u32 modelIndex, u32 meshIndex) const;
        const std::vector<std::tuple<renderer::StreamBufferDescription, renderer::DrawProperties>>& getDrawStates() const;

        static ModelHelper* createModelHelper(renderer::CommandList& cmdList, const std::vector<const Model*>& models);

    private:

        renderer::CommandList& m_cmdList;

        std::vector<const Model*> m_models;
        std::vector<std::tuple<renderer::StreamBufferDescription, renderer::DrawProperties>> m_drawState;
        std::vector<utils::IntrusivePointer<renderer::StreamBuffer>> m_buffers;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
