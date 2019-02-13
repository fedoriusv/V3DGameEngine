#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"
#include "Renderer/BufferProperties.h"

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

        ModelHelper(renderer::CommandList& cmdList, Model* model) noexcept;
        ~ModelHelper();

        void draw();

        const renderer::VertexInputAttribDescription& getVertexInputAttribDescription(u32 meshIndex) const;

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

        Model* m_model;
        std::vector<std::tuple<renderer::StreamBufferDescription, DrawProps>> m_drawState;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
