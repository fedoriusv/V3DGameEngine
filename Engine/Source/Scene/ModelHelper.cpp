#include "ModelHelper.h"
#include "Model.h"
#include "Mesh.h"
#include "Renderer/Object/StreamBuffer.h"

namespace v3d
{
namespace scene
{

ModelHelper::ModelHelper(renderer::CommandList & cmdList, const std::vector<const Model*>& models) noexcept
    : m_cmdList(cmdList)
    , m_models(models)
{
    ASSERT(!m_models.empty(), "empty list");
    for (auto model : m_models)
    {
        ASSERT(model, "nullptr");
        for (u32 meshIndex = 0; meshIndex < model->getMeshCount(); ++meshIndex)
        {
            u64 size = model->getMeshByIndex(meshIndex)->getVertexSize();
            u8* data = model->getMeshByIndex(meshIndex)->getVertexData();
            renderer::VertexStreamBuffer* vertexBuffer = cmdList.createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, size, data);

            if (model->getMeshByIndex(meshIndex)->getIndexCount() > 0)
            {
                u32 count = model->getMeshByIndex(meshIndex)->getIndexCount();
                u8* data = model->getMeshByIndex(meshIndex)->getIndexData();
                renderer::IndexStreamBuffer* indexBuffer = cmdList.createObject<renderer::IndexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, renderer::StreamIndexBufferType::IndexType_32, count, data);

                DrawProps props = { 0, count, 0, 1, true };
                m_drawState.push_back(std::make_tuple(renderer::StreamBufferDescription(indexBuffer, 0, vertexBuffer, 0, 0), props));
            }
            else
            {
                DrawProps props = { 0, model->getMeshByIndex(meshIndex)->getVertexCount(), 0, 1, false };
                m_drawState.push_back(std::make_tuple(renderer::StreamBufferDescription(vertexBuffer, 0, 0), props));
            }
        }
    }
}

ModelHelper::~ModelHelper()
{
    m_drawState.clear();
    //renderer::VertexStreamBuffer* vertexBuffer - remove by smartptr
    //renderer::IndexStreamBuffer* indexBuffer
}

const renderer::VertexInputAttribDescription& ModelHelper::getVertexInputAttribDescription(u32 modelIndex, u32 meshIndex) const
{
    ASSERT(!m_models.empty(), "empty list");
    ASSERT(m_models[modelIndex], "nullptr");
    return m_models[modelIndex]->getMeshByIndex(meshIndex)->getVertexInputAttribDesc();
}

Transform & ModelHelper::getTransform()
{
    return m_tramsform;
}

ModelHelper * ModelHelper::createModelHelper(renderer::CommandList & cmdList, const std::vector<const Model*>& models)
{
    return new ModelHelper(cmdList, models);
}

void ModelHelper::drawModel(s32 index)
{
    if (index < 0)
    {
        for (auto& buffer : m_drawState)
        {
            const DrawProps& props = std::get<1>(buffer);
            if (props._indexDraws)
            {
                m_cmdList.drawIndexed(std::get<0>(buffer), props._start, props._count, props._countInstance);
            }
            else
            {
                m_cmdList.draw(std::get<0>(buffer), props._start, props._count, props._countInstance);
            }
        }
    }
    else
    {
        const DrawProps& props = std::get<1>(m_drawState[index]);
        if (props._indexDraws)
        {
            m_cmdList.drawIndexed(std::get<0>(m_drawState[index]), props._start, props._count, props._countInstance);
        }
        else
        {
            m_cmdList.draw(std::get<0>(m_drawState[index]), props._start, props._count, props._countInstance);
        }
    }
}

} //namespace scene
} //namespace v3d
