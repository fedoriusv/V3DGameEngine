#pragma once

#include "Render.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;
    class SwapchainTexture;
    class SyncPoint;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Swapchain interface class
    */
    class Swapchain
    {
    public:

        /**
        * @brief SwapchainParams struct
        */
        struct SwapchainParams
        {
            math::Dimension2D   _size;
            Format              _format;
            u32                 _countSwapchainImages;
            bool                _vSync;

            SwapchainParams() noexcept
                : _size({ 1024U, 768U })
                , _format(Format::Format_Undefined)
                , _countSwapchainImages(3U)
                , _vSync(true)
            {
            }
        };

        /**
        * @brief frame beginFrame
        */
        virtual void beginFrame() = 0;

        /**
        * @brief frame endFrame
        */
        virtual void endFrame() = 0;

        /**
        * @brief frame presentFrame
        */
        virtual void presentFrame(SyncPoint* sync = nullptr) = 0;

        /**
        * @brief frame getSyncPoint
        */
        virtual SyncPoint* getSyncPoint() = 0;

        /**
        * @brief getCurrentFrameIndex
        * @return Current frame index
        */
        u64 getCurrentFrameIndex() const;

        /**
        * @brief getBackbufferSize
        * @return Backbuffer size
        */
        const math::Dimension2D& getBackbufferSize() const;

        /**
        * @brief getBackbufferFormat
        * @return Backbuffer image format
        */
        Format getBackbufferFormat() const;

        /**
        * @brief getUsageFlags
        */
        TextureUsageFlags getUsageFlags() const;

        /**
        * @brief getBackbuffer
        */
        SwapchainTexture* getBackbuffer() const;

    protected:

        Swapchain() noexcept;
        virtual ~Swapchain() noexcept;

        Swapchain(const Swapchain&) = delete;
        Swapchain& operator=(const Swapchain&) = delete;

        bool create(Device* device, Format format, const math::Dimension2D& size, TextureUsageFlags flags);
        void destroy();

        u64 m_frameCounter;

    private:

        struct
        {
            SwapchainTexture* _swapchain;
            math::Dimension2D _size;
            Format            _format;
            TextureUsageFlags _usageFlags;
        } m_backufferDescription;

    };

    inline u64 Swapchain::getCurrentFrameIndex() const
    {
        return m_frameCounter;
    }

    inline const math::Dimension2D& Swapchain::getBackbufferSize() const
    {
        return m_backufferDescription._size;
    }

    inline Format Swapchain::getBackbufferFormat() const
    {
        return m_backufferDescription._format;
    }

    inline SwapchainTexture* Swapchain::getBackbuffer() const
    {
        return m_backufferDescription._swapchain;
    }

    inline TextureUsageFlags Swapchain::getUsageFlags() const
    {
        return m_backufferDescription._usageFlags;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
