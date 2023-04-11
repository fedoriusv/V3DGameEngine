#pragma once

#include "Stream/Stream.h"

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ResourceDecoder base class
    */
    class ResourceDecoder
    {
    public:

        ResourceDecoder() noexcept;
        explicit ResourceDecoder(std::vector<std::string> supportedExtensions) noexcept;
        virtual ~ResourceDecoder();

        [[nodiscard]] virtual Resource* decode(const stream::Stream* stream, const std::string& name = "") const = 0;

        bool isExtensionSupported(const std::string& extension) const;
        void setSupportedExtensions(const std::vector<std::string> supportedExtensions);
        const std::vector<std::string>& getSupportedExtensions() const;

    private:

        ResourceDecoder(const ResourceDecoder&) = delete;
        ResourceDecoder& operator=(const ResourceDecoder&) = delete;

        std::vector<std::string> m_supportedExtensions;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
