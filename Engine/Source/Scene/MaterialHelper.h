#pragma once

#include "Common.h"
#include "Utils/IntrusivePointer.h"
#include "Renderer/CommandList.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Texture.h"

#include "Material.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class MaterialHelper final
    {
    public:

        MaterialHelper() = delete;
        MaterialHelper(const MaterialHelper&) = delete;
        
        explicit MaterialHelper(renderer::CommandList& cmdList, Material* material) noexcept;
        ~MaterialHelper();

        void setTextureParameter(Material::PropertyName property, renderer::Texture* texture);
        void setFloatParameter(Material::PropertyName property, f32 value);
        void setVectorParameter(Material::PropertyName property, const math::Vector4D& vector);

        template <class TTexture>
        TTexture* getTextureParameter(Material::PropertyName property) const;
        f32 getFloatParameter(Material::PropertyName property) const;
        math::Vector4D getVectorParameter(Material::PropertyName property) const;

        static MaterialHelper* createMaterialHelper(renderer::CommandList& cmdList, Material* material);
        static std::vector<MaterialHelper*> createMaterialHelpers(renderer::CommandList& cmdList, std::vector<Material*> materials);

    private:

        Material* m_material;
        std::vector<utils::IntrusivePointer<renderer::Texture>> m_textures;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TTexture>
    inline TTexture* MaterialHelper::getTextureParameter(Material::PropertyName property) const
    {
        ASSERT(m_material, "nullptr");
        static_assert(std::is_base_of<renderer::Texture, TTexture>(), "wrong type");
        return m_material->getParameter<TTexture*>(property);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
