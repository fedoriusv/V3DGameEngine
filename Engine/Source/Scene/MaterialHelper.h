#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"
#include "Renderer/Object/ShaderProgram.h"
#include "Renderer/Object/Texture.h"
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

        void setTextureParameter(MaterialHeader::Property property, renderer::Texture* texture);
        void setFloatParameter(MaterialHeader::Property property, f32 value);
        void setVectorParameter(MaterialHeader::Property property, const core::Vector4D& vector);

        template <class TTexture>
        TTexture* getTextureParameter(MaterialHeader::Property property) const;
        f32 getFloatParameter(MaterialHeader::Property property) const;
        core::Vector4D getVectorParameter(MaterialHeader::Property property) const;

        static MaterialHelper* createMaterialHelper(renderer::CommandList& cmdList, Material* material);
        static std::vector<MaterialHelper*> createMaterialHelpers(renderer::CommandList & cmdList, std::vector<Material*> materials);

    private:

        renderer::CommandList& m_cmdList;

        Material* m_material;

        std::vector<renderer::Texture*> m_textures;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TTexture>
    inline TTexture * MaterialHelper::getTextureParameter(MaterialHeader::Property property) const
    {
        ASSERT(m_material, "nullptr");
        static_assert(std::is_base_of<renderer::Texture, TTexture>());
        return static_cast<TTexture*>(m_material->getTextureParameter(property));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d