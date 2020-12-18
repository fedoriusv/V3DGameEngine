#pragma once

#include "Stream/Stream.h"
#include "AndroidCommon.h"

namespace v3d
{
namespace utils
{
    class MemoryPool;
} //namespace utils
namespace android
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    @breaf AssetStream class. Android only. Only read from .apk
    */
    class AssetStream final : public stream::Stream
    {
    public:

        AssetStream(const AssetStream&) = delete;

        AssetStream() noexcept;
        explicit AssetStream(const std::string& name, u32 mode = AASSET_MODE_UNKNOWN) noexcept;
        ~AssetStream();

        bool open(const std::string& name, u32 mode = AASSET_MODE_UNKNOWN);
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
        bool isAllocated() const;

        const u8* data() const;
        
        const std::string& getName() const;

    private:

        AAsset*         m_asset;
        std::string     m_name;

        mutable s64     m_pos;
        mutable u32     m_size;
        mutable bool    m_mapped;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace android
} //namespace v3d