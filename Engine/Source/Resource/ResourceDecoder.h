#pragma once

#include "Stream/Stream.h"

namespace v3d
{
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ResourceDecoder base class
    */
    class ResourceDecoder
    {
    public:

        struct Policy
        {
            Policy() = default;
            virtual ~Policy() = default;
        };

        ResourceDecoder() noexcept = default;

        explicit ResourceDecoder(const std::vector<std::string>& supportedExtensions) noexcept
            : m_supportedExtensions(supportedExtensions)
        {
        }

        explicit ResourceDecoder(std::vector<std::string>&& supportedExtensions) noexcept
            : m_supportedExtensions(std::move(supportedExtensions))
        {
        }
        
        virtual ~ResourceDecoder()
        {
            m_supportedExtensions.clear();
        }

        [[nodiscard]] virtual Resource* decode(const stream::Stream* stream, const Policy* policy, u32 flags = 0, const std::string& name = "") const = 0;

        bool isExtensionSupported(const std::string& extension) const;
        void setSupportedExtensions(const std::vector<std::string> supportedExtensions);
        const std::vector<std::string>& getSupportedExtensions() const;

    private:

        ResourceDecoder(const ResourceDecoder&) = delete;
        ResourceDecoder& operator=(const ResourceDecoder&) = delete;

        std::vector<std::string> m_supportedExtensions;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    inline bool ResourceDecoder::isExtensionSupported(const std::string& extension) const
    {
        auto it = std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(), extension);
        if (it != m_supportedExtensions.end())
        {
            return true;
        }

        return false;
    }

    inline void ResourceDecoder::setSupportedExtensions(const std::vector<std::string> supportedExtensions)
    {
        m_supportedExtensions.assign(supportedExtensions.cbegin(), supportedExtensions.cend());
    }

    inline const std::vector<std::string>& ResourceDecoder::getSupportedExtensions() const
    {
        return m_supportedExtensions;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace decoders
} //namespace v3d
