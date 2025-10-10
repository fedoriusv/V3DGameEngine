#pragma once

#include "Common.h"
#include "Scene/Component.h"
#include "Resource/Resource.h"

namespace v3d
{
namespace renderer
{
    class Device;
} //namespace renderer
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Skybox class
    */
    class Skybox : public Object, public resource::Resource, public ComponentBase<Skybox, Component>
    {
    public:

        /**
        * @brief SkyboxHeader meta info
        */
        struct SkyboxHeader : resource::ResourceHeader
        {
            SkyboxHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Skybox)
            {
            }
        };

        explicit Skybox(renderer::Device* device) noexcept;
        explicit Skybox(renderer::Device* device, const SkyboxHeader& header) noexcept;
        ~Skybox();

        const std::string_view getName() const;

    private:

        SkyboxHeader            m_header;
        renderer::Device* const m_device;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;
    };

    inline const std::string_view Skybox::getName() const
    {
        return m_header.getName();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::Skybox>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d