#include "MaterialHelper.h"
#include "Renderer/Shader.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Image.h"

namespace v3d
{
namespace scene
{

 MaterialHelper * MaterialHelper::createMaterialHelper(renderer::CommandList & cmdList, Material * material)
 {
     return new MaterialHelper(cmdList, material);
 }

 MaterialHelper::MaterialHelper(renderer::CommandList & cmdList, Material * material) noexcept
     : m_cmdList(cmdList)
     , m_material(material)
 {

     std::map<scene::MaterialHeader::Property, resource::Image*> images;
     for (auto& prop : m_material->getMaterialHeader()._properties)
     {
         if (prop.second._name.empty())
         {
             continue;
         }

         resource::Image* image = resource::ResourceLoaderManager::getInstance()->load<resource::Image, resource::ImageFileLoader>(prop.second._name);
         ASSERT(image, "nullptr");

        images.emplace(prop.first, image);
     }

     for (auto& iter : images)
     {
         resource::Image* image = iter.second;
         renderer::Texture2D* texture = cmdList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write, 
             image->getFormat(), core::Dimension2D(image->getDimension().width, image->getDimension().height), 1, image->getRawData());

         //material->setParameter<renderer::Texture2D>(iter.first, texture);
         material->setTextureParameter(iter.first, texture);

         m_textures.push_back(texture);
     }
 }

 MaterialHelper::~MaterialHelper()
 {
     m_textures.clear();
 }

 void MaterialHelper::setTextureParameter(MaterialHeader::Property property, renderer::Texture * texture)
 {
     ASSERT(m_material, "nullptr");
     m_material->setTextureParameter(property, texture);
 }

 void MaterialHelper::setFloatParameter(MaterialHeader::Property property, f32 value)
 {
     ASSERT(m_material, "nullptr");
     m_material->setFloatParameter(property, value);
 }

 void MaterialHelper::setVectorParameter(MaterialHeader::Property property, const core::Vector4D & vector)
 {
     ASSERT(m_material, "nullptr");
     m_material->setVectorParameter(property, vector);
 }

 renderer::Texture * MaterialHelper::getTextureParameter(MaterialHeader::Property property) const
 {
     ASSERT(m_material, "nullptr");
     return m_material->getTextureParameter(property);
 }

 f32 MaterialHelper::getFloatParameter(MaterialHeader::Property property) const
 {
     ASSERT(m_material, "nullptr");
     return m_material->getFloatParameter(property);
 }

 core::Vector4D MaterialHelper::getVectorParameter(MaterialHeader::Property property) const
 {
     ASSERT(m_material, "nullptr");
     return m_material->getVectorParameter(property);
 }

} //namespace scene
} //namespace v3d
