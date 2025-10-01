#include "ModelHandler.h"

#include "Resource/Bitmap.h"
#include "Renderer/Device.h"
#include "Utils/Logger.h"

#include "Scene/Geometry/Mesh.h"
#include "Scene/Model.h"
#include "Scene/Material.h"

#include "RenderTechniques/VertexFormats.h"

namespace v3d
{
namespace scene
{

void ModelHandler::preUpdate(f32 dt, scene::SceneData& scene)
{
    for (u32 i = 0; i < toEnumType(RenderPipelinePass::Count); ++i)
    {
        scene.m_renderLists[toEnumType(RenderPipelinePass(i))].clear();
    }

    auto callback = [](SceneNode* parent, SceneNode* node)
        {
            if (parent)
            {
                node->m_transform[toEnumType(TransformMode::Global)].setMatrix(node->m_transform[toEnumType(TransformMode::Local)].getMatrix() * parent->m_transform[toEnumType(TransformMode::Global)].getMatrix());
            }
            else
            {
                node->m_transform[toEnumType(TransformMode::Global)].setMatrix(node->m_transform[toEnumType(TransformMode::Local)].getMatrix());
            }
        };

    //group by type
    for (auto& item : scene.m_generalRenderList)
    {
        if (item->object->m_dirty)
        {
            SceneNode::forEach(item->object, callback);
            item->object->m_dirty = false;
        }

        //fructum test
        //TODO

        if (item->object->m_visible)
        {
            u32 index = toEnumType(item->passID);
            ASSERT(index < toEnumType(RenderPipelinePass::Count), "out of range");
            scene.m_renderLists[index].push_back(item);
        }
    }
}

void ModelHandler::postUpdate(f32 dt, scene::SceneData& scene)
{
    for (auto& item : scene.m_generalRenderList)
    {
        item->object->m_prevTransform = item->object->m_transform[toEnumType(TransformMode::Global)];
    }
}

 //void ModelHandler::drawInstance(renderer::CmdListRender* cmdList, Model* model)
 //{
 //    struct ModelBuffer
 //    {
 //        math::Matrix4D modelMatrix;
 //        math::Matrix4D prevModelMatrix;
 //        math::Matrix4D normalMatrix;
 //        math::float4   tint;
 //        u64            objectID;
 //        u64           _pad = 0;
 //    };

 //    //ModelBuffer constantBuffer;
 //    //constantBuffer.modelMatrix = mesh.getTransform();
 //    //constantBuffer.prevModelMatrix = mesh.getPrevTransform();
 //    //constantBuffer.normalMatrix = constantBuffer.modelMatrix.getTransposed();
 //    //constantBuffer.tint = item._material._tint;
 //    //constantBuffer.objectID = item._objectID;

 //    //cmdList->bindDescriptorSet(1,
 //    //    {
 //    //        renderer::Descriptor(renderer::Descriptor::ConstantBuffer{ &constantBuffer, 0, sizeof(constantBuffer) }, 1),
 //    //    });

 //    //DEBUG_MARKER_SCOPE(cmdList, std::format("Object {}, pipeline {}", item._objectID, m_depthPipeline->getName()), color::rgbaf::LTGREY);
 //    //renderer::GeometryBufferDesc desc(mesh.getIndexBuffer(), 0, mesh.getVertexBuffer(0), 0, sizeof(VertexFormatStandard), 0);
 //    //cmdList->drawIndexed(desc, 0, mesh.getIndexBuffer()->getIndicesCount(), 0, 0, 1);


 //    //const renderer::GeometryBufferDesc& desc = instance->_desc;
 //    //if (desc._indexBuffer.isValid())
 //    //{
 //    //    cmdList->drawIndexed(desc, instance->_offset, instance->_count, 0, instance->_instanceOffest, instance->_instancesCount);
 //    //}
 //    //else
 //    //{
 //    //    cmdList->draw(desc, instance->_offset, instance->_count, instance->_instanceOffest, instance->_instancesCount);
 //    //}
 //}

} //namespace scene
} //namespace v3d