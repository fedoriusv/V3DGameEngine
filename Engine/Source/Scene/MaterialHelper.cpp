#include "MaterialHelper.h"
#include "Renderer/Shader.h"

#include "Resource/ResourceLoaderManager.h"
#include "Resource/ImageFileLoader.h"
#include "Resource/Bitmap.h"

namespace v3d
{
namespace scene
{

MaterialHelper* MaterialHelper::createMaterialHelper(renderer::CommandList& cmdList, Material* material)
{
    return new MaterialHelper(cmdList, material);
}

std::vector<MaterialHelper*> MaterialHelper::createMaterialHelpers(renderer::CommandList& cmdList, std::vector<Material*> materials)
{
    std::vector<MaterialHelper*> materialHelpers;
    materialHelpers.reserve(materials.size());

    for (auto& material : materials)
    {
        materialHelpers.push_back(new MaterialHelper(cmdList, material));
    }
    
    return materialHelpers;
}

MaterialHelper::MaterialHelper(renderer::CommandList& cmdList, Material * material) noexcept
    : m_material(material)
{
    std::map<scene::Material::PropertyName, std::tuple<std::string, resource::Bitmap*>> images;
    for (auto& prop : m_material->getProperties())
    {
        //if (!prop.second._name.empty())
        //{
        //    resource::Bitmap* image = resource::ResourceLoaderManager::getInstance()->load<resource::Bitmap, resource::ImageFileLoader>(prop.second._name);
        //    ASSERT(image, "nullptr");

        //    images.emplace(prop.first, std::make_tuple(prop.second._name, image));
        //}
        //else
        //{
        //    //TODO
        //}
    }

    for (auto& iter : images)
    {
        resource::Bitmap* image = std::get<1>(iter.second);
        ASSERT(image->getLayersCount() == 1, "not handled");

        renderer::Texture2D* texture = cmdList.createObject<renderer::Texture2D>(renderer::TextureUsage_Sampled | renderer::TextureUsage_Shared | renderer::TextureUsage_Write,
            image->getFormat(), math::Dimension2D(image->getDimension().m_width, image->getDimension().m_height), image->getMipMapsCount(), image->getBitmap(), std::get<0>(iter.second));


        material->setParameter<renderer::Texture*>(iter.first, texture);

        //m_textures.push_back(texture);
    }
}

MaterialHelper::~MaterialHelper()
{
    m_textures.clear();
}

void MaterialHelper::setTextureParameter(Material::PropertyName property, renderer::Texture * texture)
{
    ASSERT(m_material, "nullptr");
    m_material->setParameter<renderer::Texture*>(property, texture);
}

void MaterialHelper::setFloatParameter(Material::PropertyName property, f32 value)
{
    ASSERT(m_material, "nullptr");
    m_material->setParameter<f32>(property, value);
}

void MaterialHelper::setVectorParameter(Material::PropertyName property, const math::Vector4D & vector)
{
    ASSERT(m_material, "nullptr");
    m_material->setParameter<math::Vector4D>(property, vector);
}

f32 MaterialHelper::getFloatParameter(Material::PropertyName property) const
{
    ASSERT(m_material, "nullptr");
    return m_material->getParameter<f32>(property);
}

math::Vector4D MaterialHelper::getVectorParameter(Material::PropertyName property) const
{
    ASSERT(m_material, "nullptr");
    return m_material->getParameter<math::Vector4D>(property);
}

} //namespace scene
} //namespace v3d
