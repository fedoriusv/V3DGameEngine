#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
#include "Renderer/BufferProperties.h"

#include "Scene/Transform.h"

namespace v3d
{
namespace renderer
{
    class CommandList;
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

        static ModelHelper* createModelHelper(renderer::CommandList* cmdList, const std::vector<const Model*>& models);

        explicit ModelHelper(renderer::CommandList* cmdList, const std::vector<const Model*>& models) noexcept;
        ~ModelHelper();

        void draw(renderer::CommandList* cmdList, s32 index = -1);
        u32 getDrawStatesCount() const;

        const renderer::VertexInputAttributeDescription& getVertexInputAttribDescription(u32 modelIndex, u32 meshIndex) const;
        const std::vector<std::tuple<renderer::StreamBufferDescription, renderer::DrawProperties>>& getDrawStates() const;


    private:

        ModelHelper() = delete;
        ModelHelper(const ModelHelper&) = delete;

        std::vector<const Model*> m_models;
        std::vector<std::tuple<renderer::StreamBufferDescription, renderer::DrawProperties>> m_drawState;
        std::vector<utils::IntrusivePointer<renderer::StreamBuffer>> m_buffers;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
