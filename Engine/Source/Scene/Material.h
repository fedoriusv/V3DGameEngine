#pragma once

#include "Common.h"
#include "Object.h"
#include "Component.h"
#include "Utils/StringID.h"
#include "Resource/Resource.h"
#include "Renderer/Texture.h"

namespace v3d
{
namespace renderer
{
    class Device;
} //namespace renderer
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class MaterialShadingModel : u32
    {
        Unlit,
        PBR_MetallicRoughness,
        PBR_Specular,
        Custom,
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Material class
    */
    class Material : public Object, public resource::Resource, public ComponentBase<Material, Component>
    {
    public:

        /**
        * @brief MaterialHeader struct.
        */
        struct MaterialHeader : resource::ResourceHeader
        {
            MaterialHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Material)
            {
            }
        };

        explicit Material(renderer::Device* device, MaterialShadingModel shadingModel = MaterialShadingModel::Custom) noexcept;
        explicit Material(renderer::Device* device, const MaterialHeader& header) noexcept;
        virtual ~Material() = default;

        MaterialShadingModel getShadingModel() const;

        template<typename T>
        void setProperty(const std::string& id, const T& property);

        template<typename T>
        T getProperty(const std::string& id) const;

        bool hasProperty(const std::string& id) const;

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        using Property = std::variant<std::monostate, u32, f32, math::float4, ObjectHandle>;

        MaterialHeader                                      m_header;
        renderer::Device* const                             m_device;
        std::unordered_map<std::string, Property>           m_properties;
        MaterialShadingModel                                m_shadingModel;
    };

    inline MaterialShadingModel Material::getShadingModel() const
    {
        return m_shadingModel;
    }

    template<typename T>
    inline void Material::setProperty(const std::string& id, const T& property)
    {
        m_properties[id] = Property{ property };
    }

    template<typename T>
    inline T Material::getProperty(const std::string& id) const
    {
        auto found = m_properties.find(id);
        if (found != m_properties.cend())
        {
            if (auto val = std::get_if<T>(&found->second))
            {
                return *val;
            }
            else
            {
                return T{};
            }
        }

        return T{};
    }

    inline bool Material::hasProperty(const std::string& id) const
    {
        auto found = m_properties.find(id);
        if (found != m_properties.cend())
        {
            return true;
        }

        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::Material>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d