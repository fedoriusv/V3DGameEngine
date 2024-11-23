#include "Model.h"
#include "Mesh.h"
#include "Resource/Model.h"
#include "Renderer/Device.h"

namespace v3d
{
namespace scene
{

 Model::Model() noexcept
 {
 }
 
 Model::~Model()
 {
 }

 Model* ModelHelper::createModel(renderer::Device* device, renderer::CmdListRender* cmdList, resource::ModelResource* resource)
 {
     ASSERT(resource && resource->m_loaded, "must be valid");
     Model* model = V3D_NEW(Model, memory::MemoryLabel::MemoryObject)();
     if (resource->m_content && resource::ModelResource::ModelContent_Meshes)
     {
         for (auto& geom : resource->m_geometry)
         {
             Model::Geometry geometry;
             for (auto& lod : geom._LODs)
             {
                 Mesh* mesh = MeshHelper::createStaticMesh(device, cmdList, lod);
                 if (mesh)
                 {
                     geometry._LODs.push_back(mesh);
                 }
             }

             model->m_geometry.push_back(geometry);
             
         }
     }

     if (resource->m_content && resource::ModelResource::ModelContent_Materials)
     {
        //TODO
     }

     return model;
 }

} //namespace scene
} //namespace v3d