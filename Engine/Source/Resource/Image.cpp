#include "Image.h"

namespace v3d
{
namespace resource
{

ImageHeader::ImageHeader() noexcept
    : _format(renderer::Format::Format_Undefined)
    , _dimension(core::Dimension3D())
    , _layers(1)
    , _mips(1)

    , _size(0)
    , _flipY(false)
{
}

Image::Image(ImageHeader* header) noexcept
    : Resource(header)
    , m_rawData(nullptr)
{
}

Image::~Image()
{
    if (m_stream)
    {
        if (m_rawData)
        {
            m_stream->unmap();
            m_rawData = nullptr;
        }

        delete m_stream;
        m_stream = nullptr;
    }
}

renderer::Format Image::getFormat() const
{
    return Image::getImageHeader()._format;
}

const core::Dimension3D& Image::getDimension() const
{
    return Image::getImageHeader()._dimension;
}

u32 Image::getLayersCount() const
{
    return Image::getImageHeader()._layers;
}

u32 Image::getMipMapsCount() const
{
    return Image::getImageHeader()._mips;
}

u8* Image::getRawData() const
{
    ASSERT(m_rawData, "nullptr");
    return m_rawData;
}

void Image::init(stream::Stream * stream)
{
    ASSERT(stream, "nullptr");
    m_stream = stream;
}

bool Image::load()
{
    if (m_loaded)
    {
        return true;
    }
    ASSERT(m_stream, "nullptr");
    m_stream->seekBeg(0);

    m_rawData = m_stream->map(m_stream->size());

    m_loaded = true;

    return true;
}

const ImageHeader& Image::getImageHeader() const
{
    return *static_cast<const ImageHeader*>(m_header);
}

} //namespace resource
} //namespace v3d
