#pragma once

#include "Common.h"
#include "Resource.h"
#include "Renderer/Formats.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Bitmap. Resource class
    */
    class Bitmap : public Resource
    {
    public:

        /**
        * @brief ShaderHeader struct.
        */
        struct BitmapHeader : resource::ResourceHeader
        {
            BitmapHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Bitmap)
            {
            }
        };

        Bitmap() noexcept;
        explicit Bitmap(const BitmapHeader& header) noexcept;
        ~Bitmap();

        renderer::Format getFormat() const;
        const math::Dimension3D& getDimension() const;
        u32 getLayersCount() const;
        u32 getMipmapsCount() const;

        const void* getBitmap() const;

        u32 getSize() const;

    private:

        Bitmap(const Bitmap&) = delete;
        Bitmap& operator=(const Bitmap&) = delete;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        BitmapHeader                       m_header;
        math::Dimension3D                  m_dimension;
        renderer::Format                   m_format;
        u32                                m_layers;
        u32                                m_mips;
        std::tuple<stream::Stream*, void*> m_bitmap;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline renderer::Format Bitmap::getFormat() const
    {
        return m_format;
    }

    inline const math::Dimension3D& Bitmap::getDimension() const
    {
        return m_dimension;
    }

    inline u32 Bitmap::getLayersCount() const
    {
        return m_layers;
    }

    inline u32 Bitmap::getMipmapsCount() const
    {
        return m_mips;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
