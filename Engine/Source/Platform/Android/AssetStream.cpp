#include "AssetStream.h"
#include "Utils/Logger.h"

#include "android_native_app_glue.h"

extern android_app* g_nativeAndroidApp;

namespace v3d
{
namespace android
{

AssetStream::AssetStream() noexcept
    : m_asset(nullptr)
    , m_pos(0)
    , m_size(0)
    , m_mapped(false)
{
}

AssetStream::AssetStream(const std::string& name, u32 mode) noexcept
    : m_asset(nullptr)
    , m_pos(0)
    , m_size(0)
    , m_mapped(false)
{
    if (!AssetStream::open(name, mode))
    {
        LOG_ERROR("AssetStream: load is failed %s", name.c_str());
    }
}

AssetStream::~AssetStream()
{
    AssetStream::close();
}

bool AssetStream::open(const std::string& name, u32 mode)
{
    ASSERT(g_nativeAndroidApp->activity, "nullptr");
    m_asset = AAssetManager_open(g_nativeAndroidApp->activity->assetManager, name.c_str(), mode);
    if (!m_asset)
    {
        return false;
    }

    m_name = name;
    return true;
}

void AssetStream::close()
{
    if (m_asset)
    {
        AAsset_close(m_asset);
        m_asset = nullptr;
    }

    ASSERT(!m_mapped, "still mapped");
    m_pos = 0;
    m_size = 0;
}

bool AssetStream::isOpen() const
{
    return m_asset != nullptr;
}

u32 AssetStream::read(void* buffer, u32 size, u32 count) const
{
    ASSERT(m_asset, "nullptr");
    LOG_ERROR("ShaderBinaryFileLoader: read [%u, %u]", m_pos, AssetStream::size());
    m_pos += AAsset_read(m_asset, buffer, size * count) - 1;

    LOG_ERROR("ShaderBinaryFileLoader: read after [%u, %u]", m_pos, AssetStream::size());
    ASSERT(m_pos >= 0 && m_pos < AssetStream::size(), "out of range");
    return m_pos;
}
u32 AssetStream::read(u8& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(s8& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(u16& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(s16& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(u32& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(s32& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(u64& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(s64& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(f32& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(f64& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(f80& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(bool& value) const
{
    const u32 ret = AssetStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 AssetStream::read(std::string& value) const
{
    ASSERT(m_asset, "nullptr");

    u32 size = 0;
    AssetStream::read(size);

    value.clear();
    value.resize(size);

    AssetStream::read(reinterpret_cast<void*>(value.data()), sizeof(c8), size);
    return m_pos;
}

u32 AssetStream::write(const void* buffer, u32 size, u32 count)
{
    ASSERT(false, "write is not supported");
    return 0;
}

u32 AssetStream::write(u8 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(s8 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(u16 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(s16 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(u32 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(s32 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(u64 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(s64 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(f32 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(f64 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(f80 value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(bool value)
{
    return AssetStream::write(&value, sizeof(value), 1);
}

u32 AssetStream::write(const std::string& value)
{
    ASSERT(false, "write is not supported");
    return 0;
}

void AssetStream::seekBeg(u32 offset) const
{
    ASSERT(m_asset, "nullptr");
    m_pos = AAsset_seek(m_asset, offset, SEEK_SET);
    ASSERT(m_pos >= 0 && m_pos <= AssetStream::size(), "Invalid memory pos");
}

void AssetStream::seekEnd(u32 offset) const
{
    ASSERT(m_asset, "nullptr");
    m_pos = AAsset_seek(m_asset, offset, SEEK_END);
    ASSERT(m_pos >= 0 && m_pos <= AssetStream::size(), "Invalid memory pos");
}

void AssetStream::seekCur(u32 offset) const
{
    ASSERT(m_asset, "nullptr");
    m_pos = AAsset_seek(m_asset, offset, SEEK_CUR);
    ASSERT(m_pos >= 0 && m_pos <= AssetStream::size(), "Invalid memory pos");
}

u32 AssetStream::tell() const
{
   return static_cast<u32>(m_pos);
}

u32 AssetStream::size() const
{
    if (m_size == 0)
    {
        ASSERT(m_asset, "nullptr");
        m_size = AAsset_getLength(m_asset);
    }
    return m_size;
}

u8* AssetStream::map(u32 size) const
{
    ASSERT(!m_mapped, "already mapped");
    m_mapped = true;

    ASSERT(m_asset, "nullptr");
    ASSERT(m_pos < AssetStream::size(), "out of range");
    const u8* data = reinterpret_cast<const u8*>(AAsset_getBuffer(m_asset)) + m_pos;
    return const_cast<u8*>(data);
}

void AssetStream::unmap() const
{
    ASSERT(m_mapped, "Memory not mapped");
    m_mapped = false;
}

bool AssetStream::isMapped() const
{
    return m_mapped;
}

bool AssetStream::isAllocated() const
{
    bool allocated = AAsset_isAllocated(m_asset);
    return allocated;
}

const u8* AssetStream::data() const
{
    ASSERT(m_asset, "nullptr");
    const u8* data = reinterpret_cast<const u8*>(AAsset_getBuffer(m_asset));
    return data;
}

const std::string& AssetStream::getName() const
{
    return m_name;
}

} //namespace android
} //namespace v3d