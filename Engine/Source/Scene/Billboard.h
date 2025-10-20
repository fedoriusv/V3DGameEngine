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
    * @brief Billboard class
    */
    class Billboard : public Object, public resource::Resource, public ComponentBase<Billboard, Component>
    {
    public:

        /**
        * @brief BillboardHeader meta info
        */
        struct BillboardHeader : resource::ResourceHeader
        {
            BillboardHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Billboard)
            {
            }
        };

        explicit Billboard(renderer::Device* device) noexcept;
        explicit Billboard(renderer::Device* device, const BillboardHeader& header) noexcept;
        ~Billboard();

        const std::string_view getName() const;

    private:

        BillboardHeader         m_header;
        renderer::Device* const m_device;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;
    };

    inline const std::string_view Billboard::getName() const
    {
        return m_header.getName();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::Billboard>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d