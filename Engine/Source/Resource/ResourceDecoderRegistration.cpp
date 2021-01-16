#include "ResourceDecoderRegistration.h"
#include "ResourceDecoder.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace resource
{

ResourceDecoderRegistration::ResourceDecoderRegistration() noexcept
{
}

ResourceDecoderRegistration::~ResourceDecoderRegistration()
{
    ResourceDecoderRegistration::unregisterAllDecoders();
}

bool ResourceDecoderRegistration::registerDecoder(const ResourceDecoder* decoder)
{
    auto it = std::find(m_decoders.cbegin(), m_decoders.cend(), decoder);
    if (it == m_decoders.cend())
    {
        m_decoders.push_back(decoder);
        return true;
    }

    return false;
}

bool ResourceDecoderRegistration::unregisterDecoder(const ResourceDecoder* decoder)
{
    auto it = std::find(m_decoders.cbegin(), m_decoders.cend(), decoder);
    if (it != m_decoders.end())
    {
        m_decoders.erase(std::remove(m_decoders.begin(), m_decoders.end(), *it), m_decoders.end());
        delete* it;

        return true;
    }

    return false;
}

void ResourceDecoderRegistration::unregisterAllDecoders()
{
    for (auto decoder : m_decoders)
    {
        delete decoder;
    }
    m_decoders.clear();
}

const std::vector<const ResourceDecoder*>& ResourceDecoderRegistration::getDecoders() const
{
    return m_decoders;
}


const ResourceDecoder* ResourceDecoderRegistration::findDecoder(const std::string& extension)
{
    auto predCanDecode = [extension](const ResourceDecoder* decoder) -> bool
    {
        return decoder->isExtensionSupported(extension);
    };

    auto iter = std::find_if(m_decoders.cbegin(), m_decoders.cend(), predCanDecode);
    if (iter == m_decoders.cend())
    {
        return nullptr;
    }

    return (*iter);
}

} //namespace resource
} //namespace v3d