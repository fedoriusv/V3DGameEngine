#include "ModelHelper.h"
#include "Model.h"
#include "Renderer/Object/StreamBuffer.h"

namespace v3d
{
namespace scene
{

ModelHelper::ModelHelper(renderer::CommandList & cmdList, Model* model) noexcept
    : m_cmdList(cmdList)
    , m_model(model)
{
    bool unifyMesh = false;
    if (unifyMesh)
    {
        u32 size = model->getModleHeader()._vertex._globalSize;
        u8* data = model->getMeshByIndex(0)->getVertexData();
        renderer::VertexStreamBuffer* vertexBuffer = cmdList.createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, size, data);

        if (model->getModleHeader()._indexBuffer)
        {
            u32 count = model->getModleHeader()._index._countElements;
            u8* data = model->getMeshByIndex(0)->getIndexData();
            renderer::IndexStreamBuffer* indexBuffer = cmdList.createObject<renderer::IndexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, renderer::StreamIndexBufferType::IndexType_32, count, data);

            DrawProps props = { 0, count, 0, 1, true };
            m_drawState.push_back(std::make_tuple(renderer::StreamBufferDescription(indexBuffer, 0, vertexBuffer, 0, 0), props));
        }
        else
        {
            DrawProps props = { 0, model->getModleHeader()._vertex._countElements, 0, 1, false };
            m_drawState.push_back(std::make_tuple(renderer::StreamBufferDescription(vertexBuffer, 0, 0), props));
        }
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

ModelHelper::~ModelHelper()
{
    m_drawState.clear();
    //renderer::VertexStreamBuffer* vertexBuffer - remove by smartptr
    //renderer::IndexStreamBuffer* indexBuffer
}

const renderer::VertexInputAttribDescription& ModelHelper::getVertexInputAttribDescription(u32 meshIndex) const
{
    ASSERT(m_model, "nullptr");
    return m_model->getMeshByIndex(meshIndex)->getVertexInputAttribDesc();
}

void ModelHelper::draw()
{
    for (auto& buffer : m_drawState)
    {
        const DrawProps& props = std::get<1>(buffer);
        if (props._indexDraws)
        {
            m_cmdList.draw(std::get<0>(buffer), props._start, props._count, props._countInstance);
        }
        else
        {
            m_cmdList.drawIndexed(std::get<0>(buffer), props._start, props._count, props._countInstance);
        }
    }
}

} //namespace scene
} //namespace v3d
