#include "FileStream.h"
#include "Utils/Logger.h"
#include "Utils/MemoryPool.h"

#include <filesystem>

namespace v3d
{
namespace stream
{

using namespace core;

FileStream::FileStream(utils::MemoryPool* allocator) noexcept
    : m_file(nullptr)
    , m_size(0)
    , m_open(false)
    , m_memory(nullptr)
    , m_mapped(false)

    , m_allocator(allocator)
{
}

FileStream::FileStream(const std::string& file, OpenMode openMode, utils::MemoryPool* allocator) noexcept
    : m_file(nullptr)
    , m_size(0)
    , m_open(false)
    , m_memory(nullptr)
    , m_mapped(false)

    , m_allocator(allocator)
{
    if (!open(file, openMode))
    {
        LOG_ERROR("Can not read file: %s", file.c_str());
    }
}


FileStream::~FileStream()
{
    FileStream::close();
}

bool FileStream::open(const std::string& file, OpenMode openMode)
{
    if (m_open)
    {
        return true;
    }

    const char* mode = 0;
    if ((openMode & e_in) == openMode)
    {
        mode = "rb";
    }
    else if ((openMode & e_out) == openMode)
    {
        mode = "wb";
    }
    else if ((openMode & e_app) == openMode)
    {
        mode = "ab";
    }
    else if ((openMode & (e_in | e_out)) == openMode)
    {
        mode = "r+b";
    }
    else if ((openMode & (e_out | e_app)) == openMode)
    {
        mode = "ab";
    }
    else if (openMode & e_trunc)
    {
        mode = "wb+";
    }
    else if (openMode & e_create)
    {
        mode = "wb+";
    }
    else
    {
        mode = "rb";
    }

    m_name = file;
    m_file = fopen(m_name.c_str(), mode);
    m_open = m_file != nullptr;

    return m_open;
}

void FileStream::close()
{
    if (m_file)
    {
        fclose(m_file);
        m_file = nullptr;
    }

    ASSERT(!m_mapped, "mapped");
    m_size = 0;
    m_name.clear();
    m_open = false;
}

bool FileStream::isOpen() const
{
    return m_open;
}

u32 FileStream::read(void* buffer, u32 size, u32 count) const
{
    ASSERT(m_file, "File Handler is nullptr");
    const u32 ret = static_cast<u32>(fread(buffer, size, count, m_file));
    return ret;
}

u32 FileStream::read(u8& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(s8& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(u16& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(s16& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(u32& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(s32& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(u64& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(s64& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(f32& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(f64& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(f80& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(bool& value) const
{
    const u32 ret = FileStream::read(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::read(std::string& value) const
{
    ASSERT(m_file, "File Handler is nullptr");

    fseek(m_file, 0, SEEK_END);
    m_size = static_cast<u32>(ftell(m_file));
    rewind(m_file);

    value.clear();
    value.resize(m_size);
    const u32 ret = FileStream::read(reinterpret_cast<void*>(value.data()), sizeof(c8), m_size);

    return ret;
}

u32 FileStream::write(const void* buffer, u32 size, u32 count)
{
    ASSERT(m_file, "File Handler is nullptr");
    const u32 ret = static_cast<u32>(fwrite(buffer, size, count, m_file));
    return ret;
}

u32 FileStream::write(u8 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(s8 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(u16 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(s16 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(u32 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(s32 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(u64 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(s64 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(f32 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(f64 value)
{
    const u32 ret = (u32)write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(f80 value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(bool value)
{
    const u32 ret = FileStream::write(&value, sizeof(value), 1);
    return ret;
}

u32 FileStream::write(const std::string value)
{
    const u32 strLen = static_cast<u32>(value.length());
    u32 ret = FileStream::write(&strLen, sizeof(u32), 1);
    ret += FileStream::write(value.c_str(), sizeof(c8), strLen);
    return ret;
}

void FileStream::seekBeg(u32 offset) const
{
    ASSERT(m_file, "File Handler is nullptr");
    fseek(m_file, offset, SEEK_SET);
}

void FileStream::seekEnd(u32 offset) const
{
    ASSERT(m_file, "File Handler is nullptr");
    fseek(m_file, offset, SEEK_END);
}

void FileStream::seekCur(u32 offset) const
{
    ASSERT(m_file, "File Handler is nullptr");
    fseek(m_file, offset, SEEK_CUR);
}

u32 FileStream::tell() const
{
    ASSERT(m_file, "File Handler is nullptr");
    u32 tell = static_cast<u32>(ftell(m_file));
    return tell;
}

u32 FileStream::size() const
{
    ASSERT(m_file, "File Handler nullptr");
    if (m_size == 0)
    {
        u32 currentPos = FileStream::tell();
        fseek(m_file, 0, SEEK_END);
        m_size = FileStream::tell();
        fseek(m_file, currentPos, SEEK_SET);
    }
    return m_size;
}

u8* FileStream::map(u32 size) const
{
    ASSERT(!m_mapped, "already mapped");
    if (m_mapped)
    {
        return m_memory;
    }

    ASSERT(size > 0 && FileStream::tell() + size <= FileStream::size(), "Invalid file size");
    if (m_allocator)
    {
        m_memory = reinterpret_cast<u8*>(m_allocator->getMemory(size));
    }
    else
    {
        m_memory = new u8[size];
    }

    FileStream::read(m_memory, size);
    m_mapped = true;

    return m_memory;
}

void FileStream::unmap() const
{
    ASSERT(m_mapped, "Memory not mapped");
    if (m_memory)
    {
        if (m_allocator)
        {
            m_allocator->freeMemory(m_memory);
        }
        else
        {
            delete[] m_memory;
            m_memory = nullptr;
        }
    }
    m_mapped = false;
}

bool FileStream::isMapped() const
{
    return m_mapped;
}

const std::string& FileStream::getName() const
{
    return m_name;
}

bool FileStream::isExists(const std::string& file)
{
#ifndef _ANDROID
    return std::filesystem::exists(file);
#else
    ASSERT(false, "not implemented");
    return false;
#endif //_ANDROID
}

std::string FileStream::absolutePath(const std::string & file)
{
#ifndef _ANDROID
    std::filesystem::path path = file;
    return std::filesystem::absolute(path).string();
#else
    ASSERT(false, "not implemented");
    return std::string("");
#endif //_ANDROID
}

std::string FileStream::extension(const std::string & file)
{
#ifndef _ANDROID
    std::filesystem::path path = file;
    return path.extension().string();
#else
    ASSERT(false, "not implemented");
    return std::string("");
#endif //_ANDROID
}

bool FileStream::isDirectory(const std::string& path)
{
#ifndef _ANDROID
    return std::filesystem::is_directory(path);
#else
    ASSERT(false, "not implemented");
    return false;
#endif //_ANDROID
}

bool FileStream::remove(const std::string& file)
{
#ifndef _ANDROID
    return std::filesystem::remove(file.c_str());
#else
    ASSERT(false, "not implemented");
    return false;
#endif //_ANDROID
}

} //namespace stream
} //namespace v3d
