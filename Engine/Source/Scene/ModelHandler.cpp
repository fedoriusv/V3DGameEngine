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