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
    * ImageHeader struct.
    */
    struct ImageHeader : ResourceHeader
    {
        ImageHeader() noexcept;

        renderer::Format    _format;
        core::Dimension3D   _dimension;
        u32                 _layers;
        u32                 _mips;

        u64                 _size;

        bool                _flipY;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Image. Resource class
    */
    class Image : public Resource
    {
    public:

        explicit Image(ImageHeader* header) noexcept;
        ~Image();

        renderer::Format getFormat() const;
        core::Dimension3D getDimension() const;
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