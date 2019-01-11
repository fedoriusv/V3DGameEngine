#include "FileStream.h"
#include "utils/Logger.h"

#include <filesystem>

namespace v3d
{
namespace stream
{

using namespace core;

FileStream::FileStream() noexcept
    : m_fileHandler(nullptr)
    , m_fileSize(0)
    , m_isOpen(false)
    , m_mappedMemory(nullptr)
    , m_mapped(false)
{
}

FileStream::FileStream(const std::string& file, OpenMode openMode) noexcept
    : m_fileHandler(nullptr)
    , m_fileSize(0)
    , m_isOpen(false)
    , m_mappedMemory(nullptr)
    , m_mapped(false)
{
    open(file, openMode);

    if (!m_isOpen)
    {
        LOG_ERROR("Can not read file: %s", file.c_str());
    }
}


FileStream::~FileStream()
{
    close();
}

bool FileStream::open(const std::string& file, OpenMode openMode)
{
    if (m_isOpen)
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

    m_fileName = file;
    m_fileHandler = fopen(m_fileName.c_str(), mode);
 
    m_isOpen = m_fileHandler != nullptr;

    return m_isOpen;
}

void FileStream::close()
{
    if (m_fileHandler)
    {
        fclose(m_fileHandler);
        m_fileHandler = nullptr;
    }
    m_fileSize = 0;
    m_fileName = "";
    m_isOpen = false;
}


bool FileStream::isOpen() const
{
    return m_isOpen;
}

u32 FileStream::read(void* buffer, u32 size, u32 count) const
{
    ASSERT(m_fileHandler, "File Handler nullptr");
    const u32 ret = (u32)fread(buffer, size, count, m_fileHandler);
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
    ASSERT(m_fileHandler, "File Handler nullptr");

    fseek(m_fileHandler, 0, SEEK_END);
    m_fileSize = ftell(m_fileHandler);
    rewind(m_fileHandler);

    value.clear();
    value.resize(m_fileSize);
    const u32 ret = FileStream::read((void*)value.data(), sizeof(c8), m_fileSize);

    return ret;
}

u32 FileStream::write(const void* buffer, u32 size, u32 count)
{
    ASSERT(m_fileHandler, "File Handler nullptr");
    const u32 ret = (u32)fwrite(buffer, size, count, m_fileHandler);
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
    const u32 strLen = (u32)value.length();
    u32 ret = FileStream::write(&strLen, sizeof(u32), 1);
    ret += FileStream::write(value.c_str(), sizeof(c8), strLen);
    return ret;
}

void FileStream::seekBeg(u32 offset) const
{
    ASSERT(m_fileHandler, "File Handler nullptr");
    fseek(m_fileHandler, offset, SEEK_SET);
}

void FileStream::seekEnd(u32 offset) const
{
    ASSERT(m_fileHandler, "File Handler nullptr");
    fseek(m_fileHandler, offset, SEEK_END);
}

void FileStream::seekCur(u32 offset) const
{
    ASSERT(m_fileHandler, "File Handler nullptr");
    fseek(m_fileHandler, offset, SEEK_CUR);
}

u32 FileStream::tell() const
{
    ASSERT(m_fileHandler, "File Handler nullptr");
    u32 tell = ftell(m_fileHandler);
    return tell;
}

u32 FileStream::size() const
{
    ASSERT(m_fileHandler, "File Handler nullptr");
    if (m_fileSize == 0)
    {
        u32 currentPos = tell();
        fseek(m_fileHandler, 0, SEEK_END);
        m_fileSize = tell();
        fseek(m_fileHandler, currentPos, SEEK_SET);
    }
    return m_fileSize;
}

u8* FileStream::map(u32 size)
{
    ASSERT(size > 0 && tell() + size <= FileStream::size(), "Invalid file size");
    u8* address = 0;

     m_mappedMemory = new u8[size];
     address = m_mappedMemory;

    read(address, size);

    ASSERT(!m_mapped, "Memory not mapped");
    m_mapped = true;

    return address;
}

void FileStream::unmap()
{
    if (m_mappedMemory)
    {
        delete[] m_mappedMemory;
        m_mappedMemory = nullptr;
    }
    ASSERT(m_mapped, "Memory not mapped");
    m_mapped = false;
}

const std::string& FileStream::getName() const
{
    return m_fileName;
}

bool FileStream::isExists(const std::string& file)
{
    return std::filesystem::exists(file);
}

bool FileStream::isDirectory(const std::string& path)
{
    return std::filesystem::is_directory(path);
}

bool FileStream::remove(const std::string& file)
{
    return std::filesystem::remove(file.c_str());
}

} //namespace stream
} //namespace v3d
