#include "Geometry.h"
#include "Stream/StreamManager.h"
#include "Scene/Model.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

GeometryHeader::GeometryHeader() noexcept
    : _size(0)
    , _offset(0)
    , _flags(0)
{
}

GeometryHeader::GeometryInfo::GeometryInfo()
    : _count(0)
    , _size(0)
{
}

void GeometryHeader::GeometryInfo::operator>>(stream::Stream* stream) const
{
    stream->write<u32>(_count);
    stream->write<u64>(_size);

    u32 count = static_cast<u32>(_subData.size());
    stream->write<u32>(count);
    stream->write(_subData.data(), count * sizeof(SubData), 1);
}

void GeometryHeader::GeometryInfo::operator<<(const stream::Stream* stream)
{
    stream->read<u32>(_count);
    stream->read<u64>(_size);

    u32 count;
    stream->read<u32>(count);
    _subData.resize(count);
    stream->read(_subData.data(), count * sizeof(SubData), 1);
}

Geometry::SubGeometry::SubGeometry() noexcept
    : m_indexCount(0)
    , m_vertexCount(0)
{
    memset(&m_indexData, 0, sizeof(BufferData));
    memset(&m_vertexData, 0, sizeof(BufferData));
}

u8* Geometry::SubGeometry::getVertexData() const
{
    ASSERT(m_vertexData._data, "nullptr");
    return m_vertexData._data;
}

u32 Geometry::SubGeometry::getVertexCount() const
{
    return m_vertexCount;
}

u64 Geometry::SubGeometry::getVertexSize() const
{
    ASSERT(m_vertexData._data, "nullptr");
    return m_vertexData._size;
}

u8* Geometry::SubGeometry::getIndexData() const
{
    ASSERT(m_indexData._data, "nullptr");
    return m_indexData._data;
}

u32 Geometry::SubGeometry::getIndexCount() const
{
    return m_indexCount;
}

u64 Geometry::SubGeometry::getIndexSize() const
{
    ASSERT(m_indexData._data, "nullptr");
    return m_indexData._size;
}

void Geometry::SubGeometry::fillVertexData(u32 count, u8 * data, u64 size)
{
    ASSERT(!m_vertexData._data, "not nullptr");
    m_vertexCount = count;
    m_vertexData._data = data;
    m_vertexData._size = size;
}


void Geometry::SubGeometry::fillIndexData(u32 count, u8 * data, u64 size)
{
    ASSERT(!m_indexData._data, "not nullptr");
    m_indexCount = count;
    m_indexData._data = data;
    m_indexData._size = size;
}

Geometry::Geometry(GeometryHeader* header) noexcept
    : Resource(header)
    , m_indexStreamBuffer(nullptr)
    , m_vertexStreamBuffer(nullptr)

    , m_indexCount(0)
    , m_vertexCount(0)
{
    LOG_DEBUG("Geometry constructor %xll", this);

    memset(&m_indexData, 0, sizeof(BufferData));
    memset(&m_vertexData, 0, sizeof(BufferData));
}

Geometry::~Geometry()
{
    LOG_DEBUG("Geometry destructor %xll", this);

    for (auto& geom : m_subGeometry)
    {
        delete geom;
    }
    m_subGeometry.clear();

    if (m_vertexStreamBuffer)
    {
        delete m_vertexStreamBuffer;
        m_vertexStreamBuffer = nullptr;
    }

    if (m_indexStreamBuffer)
    {
        delete m_indexStreamBuffer;
        m_indexStreamBuffer = nullptr;
    }
}


void Geometry::init(stream::Stream* stream)
{
    ASSERT(stream, "nullptr");
    m_stream = stream;
}

bool Geometry::load()
{
    if (m_loaded)
    {
        return true;
    }
    ASSERT(m_stream, "nullptr");
    m_stream->seekBeg(0);

    m_description << m_stream;

    const GeometryHeader::GeometryInfo& vertexInfo = Geometry::getGeometryHeader()._vertex;
    const GeometryHeader::GeometryInfo& indexInfo = Geometry::getGeometryHeader()._index;

    m_vertexStreamBuffer = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(vertexInfo._size));
    u8* vetexData = m_vertexStreamBuffer->map(static_cast<u32>(vertexInfo._size));
    m_stream->read(vetexData, static_cast<u32>(vertexInfo._size), 1);
    Geometry::fillVertexData(vertexInfo._count, vetexData, vertexInfo._size);

    m_subGeometry.resize(static_cast<u32>(vertexInfo._subData.size()));

    u8* indexData = nullptr;
    if (Geometry::getGeometryHeader()._flags & GeometryHeader::GeometryFlag_PresentIndex)
    {
        m_indexStreamBuffer = stream::StreamManager::createMemoryStream(nullptr, static_cast<u32>(indexInfo._size));
        indexData = m_indexStreamBuffer->map(static_cast<u32>(indexInfo._size));
        m_stream->read(indexData, static_cast<u32>(indexInfo._size), 1);
        m_indexStreamBuffer->unmap();

        Geometry::fillIndexData(indexInfo._count, indexData, indexInfo._size);
    }

    for (u32 geomIndex = 0; geomIndex < m_subGeometry.size(); ++geomIndex)
    {
        SubGeometry* geometry = new SubGeometry();

        u8* vertexDataPtr = vetexData + vertexInfo._subData[geomIndex]._offset;
        u64 vertexSize = vertexInfo._subData[geomIndex]._size;
        geometry->fillVertexData(vertexInfo._subData[geomIndex]._count, vertexDataPtr, vertexSize);

        if (Geometry::getGeometryHeader()._flags & GeometryHeader::GeometryFlag_PresentIndex)
        {
            u8* indexDataPtr = indexData + indexInfo._subData[geomIndex]._offset;
            u64 indexSize = indexInfo._subData[geomIndex]._size;
            geometry->fillIndexData(indexInfo._subData[geomIndex]._count, indexDataPtr, indexSize);
        }

        m_subGeometry[geomIndex] = geometry;
    }
    m_vertexStreamBuffer->unmap();

    ASSERT(!m_stream->isMapped(), "mapped");
    delete m_stream;
    m_stream = nullptr;

    m_loaded = true;

    return true;
}

const renderer::VertexInputAttribDescription& Geometry::getVertexInputAttribDesc() const
{
    return m_description;
}

u8* Geometry::getVertexData(s32 subGeometry) const
{
    ASSERT(m_subGeometry.size() > subGeometry || subGeometry < 0, "out of range");
    if (subGeometry < 0)
    {
        ASSERT(m_vertexData._data, "nullptr");
        return m_vertexData._data;
    }
    return m_subGeometry[subGeometry]->getVertexData();
}

u32 Geometry::getVertexCount(s32 subGeometry) const
{
    ASSERT(m_subGeometry.size() > subGeometry || subGeometry < 0, "out of range");
    if (subGeometry < 0)
    {
        return m_vertexCount;
    }
    return m_subGeometry[subGeometry]->getVertexCount();
}

u64 Geometry::getVertexSize(s32 subGeometry) const
{
    ASSERT(m_subGeometry.size() > subGeometry || subGeometry < 0, "out of range");
    if (subGeometry < 0)
    {
        ASSERT(m_vertexData._data, "nullptr");
        return m_vertexData._size;
    }
    return m_subGeometry[subGeometry]->getVertexSize();
}

u8* Geometry::getIndexData(s32 subGeometry) const
{
    ASSERT(m_subGeometry.size() > subGeometry || subGeometry < 0, "out of range");
    if (subGeometry < 0)
    {
        ASSERT(m_indexData._data, "nullptr");
        return m_indexData._data;
    }
    return m_subGeometry[subGeometry]->getIndexData();
}

u32 Geometry::getIndexCount(s32 subGeometry) const
{
    ASSERT(m_subGeometry.size() > subGeometry || subGeometry < 0, "out of range");
    if (subGeometry < 0)
    {
        return m_indexCount;
    }
    return m_subGeometry[subGeometry]->getVertexCount();
}

u64 Geometry::getIndexSize(s32 subGeometry) const
{
    ASSERT(m_subGeometry.size() > subGeometry || subGeometry < 0, "out of range");
    if (subGeometry < 0)
    {
        ASSERT(m_indexData._data, "nullptr");
        return m_indexData._size;
    }
    return m_subGeometry[subGeometry]->getIndexSize();
}

const GeometryHeader& Geometry::getGeometryHeader() const
{
    return *static_cast<const GeometryHeader*>(m_header);
}

void Geometry::fillVertexData(u32 count, u8* data, u64 size)
{
    ASSERT(!m_vertexData._data, "not nullptr");
    m_vertexCount = count;
    m_vertexData._data = data;
    m_vertexData._size = size;
}

void Geometry::fillIndexData(u32 count, u8* data, u64 size)
{
    ASSERT(!m_indexData._data, "not nullptr");
    m_indexCount = count;
    m_indexData._data = data;
    m_indexData._size = size;
}

} //namespace scene
} //namespace v3d
