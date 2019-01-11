#pragma once

#include "Stream/Stream.h"
#include "Utils/NonCopyable.h"

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Base class ResourceDecoder
    */
    class ResourceDecoder : public utils::NonCopyable
    {
    public:

        ResourceDecoder() noexcept;
        explicit ResourceDecoder(std::vector<std::string> supportedExtensions) noexcept;
        virtual  ~ResourceDecoder();

        virtual Resource* decode(const stream::Stream* stream, const std::string& name = "") = 0;

        bool isExtensionSupported(const std::string& extension) const;
        void setSupportedExtensions(const std::vector<std::string> supportedExtensions);
        const std::vector<std::string>& getSupportedExtensions() const;

    protected:

        std::vector<std::string> m_supportedExtensions;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
