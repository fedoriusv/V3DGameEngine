#pragma once

#include "Stream.h"

namespace v3d
{
namespace stream
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * MemoryStream class
    */
    class MemoryStream : public Stream
    {
    public:

        MemoryStream() noexcept;
        MemoryStream(const MemoryStream& stream) noexcept;
        MemoryStream(MemoryStream&& stream) noexcept;
        MemoryStream(const void* data, u32 size) noexcept;
        ~MemoryStream();

        void clear();
        void allocate(u32 size);

        void close() override;

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
        u32 write(u8 value)  override;
        u32 write(s8 value)  override;
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
        u32 write(const std::string value) override;

        void seekBeg(u32 offset) const override;
        void seekEnd(u32 offset) const override;
        void seekCur(u32 offset) const override;
        u32  tell() const override;
        u32  size() const override;

        u8* map(u32 size) override;
        void unmap() override;

        u8* getData() const;

    private:

        bool checkSize(u32 size);

        u8*             m_stream;
        u32             m_length;
        u32             m_allocated;
        mutable u32     m_pos;
        bool            m_mapped;
         
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d
