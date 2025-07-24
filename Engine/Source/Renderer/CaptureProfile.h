#pragma once

#include "Common.h"
#include "Platform/Window.h"

namespace v3d
{
namespace renderer
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CaptureProfile interface
    */
    class CaptureProfile
    {
    public:

        CaptureProfile() = default;
        virtual ~CaptureProfile() = default;

        virtual bool create() = 0;
        virtual void destroy() = 0;

        virtual void beginCapture(platform::Window* window) = 0;
        virtual void endCapture(platform::Window* window) = 0;
        virtual void capture(const std::string& file) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if ENABLE_RENDERDOC_PROFILE
    class CaptureRenderdocProfile : public CaptureProfile
    {
    public:

        CaptureRenderdocProfile() = default;
        ~CaptureRenderdocProfile() = default;

        bool create() final;
        void destroy() final;

        void beginCapture(platform::Window* window) final;
        void endCapture(platform::Window* window) final;
        void capture(const std::string& file) final;

        void showOverlay(bool show);

    private:

        bool m_loaded = false;
        bool m_capturing = false;
    };
#endif //ENABLE_RENDERDOC_PROFILE

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if ENABLE_PIX_PROFILE
    class CapturePIXProfile : public CaptureProfile
    {
    public:

        CapturePIXProfile() = default;
        ~CapturePIXProfile() = default;

        bool create() final;
        void destroy() final;

        void beginCapture(platform::Window* window) final;
        void endCapture(platform::Window* window) final;
        void capture(const std::string& file) final;

        void showOverlay(bool show);

    private:

        bool m_loaded = false;
    };
#endif //ENABLE_PIX_PROFILE

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d