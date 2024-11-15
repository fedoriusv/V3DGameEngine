#include "Bitmap.h"
#include "Stream/Stream.h"
#include "Utils/Logger.h"
#include "Stream/StreamManager.h"

namespace v3d
{
namespace resource
{
Bitmap::Bitmap() noexcept
    : m_header()
    , m_dimension(math::Dimension3D())
    , m_format(renderer::Format::Format_Undefined)
    , m_layers(0)
    , m_mips(0)
    , m_bitmap({ nullptr, nullptr })
{
    LOG_DEBUG("Bitmap constructor %llx", this);
}

Bitmap::Bitmap(const BitmapHeader& header) noexcept
    : m_header(header)
    , m_dimension(math::Dimension3D())
    , m_format(renderer::Format::Format_Undefined)
    , m_layers(0)
    , m_mips(0)
    , m_bitmap({ nullptr, nullptr })
{
    LOG_DEBUG("Bitmap constructor %llx", this);
}

Bitmap::~Bitmap()
{
    LOG_DEBUG("Bitmap destructor %llx", this);

    if (std::get<0>(m_bitmap))
    {
        std::get<0>(m_bitmap)->unmap();
        stream::StreamManager::destroyStream(std::get<0>(m_bitmap));
        m_bitmap = { nullptr, nullptr };
    }
}

bool Bitmap::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("Bitmap::load: the bitmap %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "stream is nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    stream->read<math::Dimension3D>(m_dimension);
    stream->read<renderer::Format>(m_format);
    stream->read<u32>(m_layers);
    stream->read<u32>(m_mips);

    u32 size = 0;
    stream->read<u32>(size);
    if (size > 0)
    {
        stream::Stream* bitmap = stream::StreamManager::createMemoryStream(nullptr, size);
        void* ptr = bitmap->map(size);
        stream->read(ptr, size);

        m_bitmap = { bitmap, ptr };
    }
    LOG_DEBUG("Bitmap::load: The stream has been read %d from %d bytes", stream->tell() - m_header._offset, m_header._size);

    m_loaded = true;
    return true;
}

bool Bitmap::save(stream::Stream* stream, u32 offset) const
{
    if (!m_loaded)
    {
        LOG_WARNING("Bitmap::save: the bitmap %llx is not loaded", this);
        return false;
    }

    stream->write<math::Dimension3D>(m_dimension);
    stream->write<renderer::Format>(m_format);
    stream->write<u32>(m_layers);
    stream->write<u32>(m_mips);

    //TODO
    NOT_IMPL;

    return false;
}

const void* Bitmap::getBitmap() const
{
    ASSERT(std::get<1>(m_bitmap), "nullptr");
    return std::get<1>(m_bitmap);
}

u32 Bitmap::getSize() const
{
    ASSERT(std::get<0>(m_bitmap), "nullptr");
    return std::get<0>(m_bitmap)->size();
}

} //namespace resource
} //namespace v3d
