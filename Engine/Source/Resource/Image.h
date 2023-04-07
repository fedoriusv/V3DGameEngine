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
    * @brief ImageHeader struct.
    */
    struct ImageHeader : ResourceHeader
    {
        ImageHeader() noexcept;

        renderer::Format    _format;
        math::Dimension3D   _dimension;
        u32                 _layers;
        u32                 _mips;

        bool                _flipY;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Image. Resource class
    */
    class Image : public Resource
    {
    public:

        explicit Image(ImageHeader* header) noexcept;
        ~Image();

        renderer::Format getFormat() const;
        const math::Dimension3D& getDimension() const;
        u32 getLayersCount() const;
        u32 getMipMapsCount() const;

        u8* getRawData() const;

        void init(stream::Stream* stream) override;
        bool load() override;

    private:

        const ImageHeader& getImageHeader() const;

        u8* m_rawData;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
