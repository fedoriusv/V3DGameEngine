#pragma once

#include "Common.h"

namespace v3d
{
namespace stream
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Base Interface for Stream class
    */
    class Stream
    {
    public:
        Stream() {}
        virtual ~Stream() {}

        template <class T>
        u32 read(T& value) const;

        virtual u32 read(void* buffer, u32 size, u32 count = 1) const = 0;
        virtual u32 read(u8& value) const = 0;
        virtual u32 read(s8& value) const = 0;
        virtual u32 read(u16& value) const = 0;
        virtual u32 read(s16& value) const = 0;
        virtual u32 read(u32& value) const = 0;
        virtual u32 read(s32& value) const = 0;
        virtual u32 read(u64& value) const = 0;
        virtual u32 read(s64& value) const = 0;
        virtual u32 read(f32& value) const = 0;
        virtual u32 read(f64& value) const = 0;
        virtual u32 read(f80& value) const = 0;
        virtual u32 read(bool& value) const = 0;
        virtual u32 read(std::string& value) const = 0;

        template <class T>
        u32 write(T& value);

        virtual u32 write(const void* buffer, u32 size, u32 count = 1) = 0;
        virtual u32 write(u8 value) = 0;
        virtual u32 write(s8 value) = 0;
        virtual u32 write(u16 value) = 0;
        virtual u32 write(s16 value) = 0;
        virtual u32 write(u32 value) = 0;
        virtual u32 write(s32 value) = 0;
        virtual u32 write(u64 value) = 0;
        virtual u32 write(s64 value) = 0;
        virtual u32 write(f32 value) = 0;
        virtual u32 write(f64 value) = 0;
        virtual u32 write(f80 value)  = 0;
        virtual u32 write(bool value) = 0;
        virtual u32 write(const std::string value) = 0;

        virtual void close() = 0;

        virtual void seekBeg(u32 offset) const = 0;
        virtual void seekEnd(u32 offset) const = 0;
        virtual void seekCur(u32 offset) const = 0;

        virtual u32  tell() const = 0;
        virtual u32  size() const = 0;

        virtual u8*  map(u32 size) = 0;
        virtual void unmap() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    u32 Stream::read(T& value) const
    {
        const u32 ret = read(&value, sizeof(T), 1);
        return ret;
    }

    template <class T>
    u32 Stream::write(T& value)
    {
        const u32 ret = write(&value, sizeof(T), 1);
        return ret;
    }

    template <class T>
    Stream& operator >> (Stream& stream, T& value)
    {
        stream.read(value);
        return stream;
    }

    template <class T>
    Stream& operator << (Stream& stream, const T& value)
    {
        stream.write(value);
        return stream;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d
