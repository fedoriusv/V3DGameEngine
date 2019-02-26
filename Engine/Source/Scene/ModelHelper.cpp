#include "ModelHelper.h"
#include "Model.h"
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
        bool unifyMesh = true;
        if (unifyMesh)
        {
            u32 size = static_cast<u32>(model->getModelHeader()._vertex._globalSize);
            u8* data = model->getMeshByIndex(0)->getVertexData();
            renderer::VertexStreamBuffer* vertexBuffer = cmdList.createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, size, data);

            if (model->getModelHeader()._index._present)
            {
                u32 count = static_cast<u32>(model->getModelHeader()._index._countElements);
                u8* data = model->getMeshByIndex(0)->getIndexData();
                renderer::IndexStreamBuffer* indexBuffer = cmdList.createObject<renderer::IndexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, renderer::StreamIndexBufferType::IndexType_32, count, data);

                DrawProps props = { 0, count, 0, 1, true };
                m_drawState.push_back(std::make_tuple(renderer::StreamBufferDescription(indexBuffer, 0, vertexBuffer, 0, 0), props));
            }
            else
            {
                DrawProps props = { 0, static_cast<u32>(model->getModelHeader()._vertex._countElements), 0, 1, false };
                m_drawState.push_back(std::make_tuple(renderer::StreamBufferDescription(vertexBuffer, 0, 0), props));
            }
        }
        else
        {
            ASSERT(false, "not implemented");
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

void ModelHelper::drawModel()
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

} //namespace scene
} //namespace v3d
