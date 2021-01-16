#pragma once

#include "Stream/Stream.h"

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Base class ResourceDecoder
    */
    class ResourceDecoder
    {
    public:

        ResourceDecoder(const ResourceDecoder&) = delete;
        ResourceDecoder& operator=(const ResourceDecoder&) = delete;

        ResourceDecoder() noexcept;
        explicit ResourceDecoder(std::vector<std::string> supportedExtensions) noexcept;
        virtual  ~ResourceDecoder();

        virtual Resource* decode(const stream::Stream* stream, const std::string& name = "") const = 0;

        bool isExtensionSupported(const std::string& extension) const;
        void setSupportedExtensions(const std::vector<std::string> supportedExtensions);
        const std::vector<std::string>& getSupportedExtensions() const;

    private:

        std::vector<std::string> m_supportedExtensions;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
