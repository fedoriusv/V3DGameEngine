#pragma once

#include "Stream.h"

namespace v3d
{
namespace utils
{
    class MemoryPool;
} //namespace utils
namespace stream
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * FileStream class
    */
    class FileStream final : public Stream
    {
    public:

        enum OpenMode
        {
            e_in     = 1 << 0,
            e_out    = 1 << 1,
            e_ate    = 1 << 2,
            e_app    = 1 << 3,
            e_trunc  = 1 << 4,
            e_create = 1 << 5
        };

        static std::string absolutePath(const std::string& file);
        static std::string extension(const std::string& file);

        static bool isExists(const std::string& file);
        static bool isDirectory(const std::string& path);
        static bool remove(const std::string& file);

        FileStream() noexcept = delete;
        FileStream(const FileStream&) = delete;

        FileStream(utils::MemoryPool* allocator = nullptr) noexcept;
        explicit FileStream(const std::string& file, OpenMode openMode = e_in, utils::MemoryPool* allocator = nullptr) noexcept;
        ~FileStream();

        bool open(const std::string& file, OpenMode openMode = e_in);
        void close() override;

        bool isOpen() const;

        u32 read(void* buffer, u32 size, u32 count = 1) const override;
        u32 read(u8& value) const override;
        u32 read(s8& value) const override;
        u32 read(u16& value) const override;
        u32 read(s16& value) const override;
        u32 read(u32& value) const override;
        u32 read(s32& value) const override;
        u32 read(u64& value) const override;
        u32 read(s64& value) const override;
        u32 read(f32& value) const override;
        u32 read(f64& value) const override;
        u32 read(f80& value) const override;
        u32 read(bool& value) const override;
        u32 read(std::string& value) const override;

        u32 write(const void* buffer, u32 size, u32 count = 1) override;
        u32 write(u8 value) override;
        u32 write(s8 value) override;
        u32 write(u16 value) override;
        u32 write(s16 value) override;
        u32 write(u32 value) override;
        u32 write(s32 value) override;
        u32 write(u64 value) override;
        u32 write(s64 value) override;
        u32 write(f32 value) override;
        u32 write(f64 value) override;
        u32 write(f80 value) override;
        u32 write(bool value) override;
        u32 write(const std::string& value) override;
        
        void seekBeg(u32 offset) const override;
        void seekEnd(u32 offset) const override;
        void seekCur(u32 offset) const override;
        u32 tell() const override;
        u32 size() const override;

        u8* map(u32 size) const override;
        void unmap() const override;
        bool isMapped() const override;
        
        const std::string& getName() const;

    protected:

        FILE*           m_file;
        std::string     m_name;

        mutable u32     m_size;
        bool            m_open;

        mutable u8*     m_memory;
        mutable bool    m_mapped;

        utils::MemoryPool* const m_allocator;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d
