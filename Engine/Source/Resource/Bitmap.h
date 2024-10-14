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
    * @brief BitmapHeader struct.
    */
    struct BitmapHeader : ResourceHeader
    {
        enum BitmapHeaderFlag
        {
            BitmapFlippedByY = 1 << 0,
        };
        typedef u32 BitmapHeaderFlags;

        BitmapHeader() noexcept;
        BitmapHeader(const BitmapHeader& other) noexcept;
        ~BitmapHeader() = default;

        u32 operator>>(stream::Stream* stream) const;
        u32 operator<<(const stream::Stream* stream);

        math::Dimension3D   _dimension;
        renderer::Format    _format;
        u16                 _layers;
        u16                 _mips;
        BitmapHeaderFlags   _bitmapFlags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Bitmap. Resource class
    */
    class Bitmap : public Resource
    {
    public:

        Bitmap() noexcept;
        explicit Bitmap(BitmapHeader* header) noexcept;
        ~Bitmap();

        renderer::Format getFormat() const;
        const math::Dimension3D& getDimension() const;
        u32 getLayersCount() const;
        u32 getMipMapsCount() const;

        const void* getBitmap() const;
        u32 getSize() const;

    private:

        Bitmap(const Bitmap&) = delete;
        Bitmap& operator=(const Bitmap&) = delete;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        BitmapHeader* m_header;

        std::tuple<stream::Stream*, void*> m_bitmap;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline renderer::Format Bitmap::getFormat() const
    {
        ASSERT(m_header, "nullptr");
        return m_header->_format;
    }

    inline const math::Dimension3D& Bitmap::getDimension() const
    {
        ASSERT(m_header, "nullptr");
        return m_header->_dimension;
    }

    inline u32 Bitmap::getLayersCount() const
    {
        ASSERT(m_header, "nullptr");
        return m_header->_layers;
    }

    inline u32 Bitmap::getMipMapsCount() const
    {
        ASSERT(m_header, "nullptr");
        return m_header->_mips;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
