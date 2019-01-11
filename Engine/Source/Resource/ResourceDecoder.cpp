#include "ResourceDecoder.h"

namespace v3d
{
namespace resource
{

ResourceDecoder::ResourceDecoder()
{
}

ResourceDecoder::ResourceDecoder(std::vector<std::string> supportedExtensions)
    : m_supportedExtensions(supportedExtensions)
{
}

ResourceDecoder::~ResourceDecoder()
{
    m_supportedExtensions.clear();
}

bool ResourceDecoder::isExtensionSupported(const std::string & extension) const
{
    auto it = std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(), extension);
    if (it != m_supportedExtensions.end())
    {
        return true;
    }

    return false;
}

void ResourceDecoder::setSupportedExtensions(const std::vector<std::string> supportedExtensions)
{
    m_supportedExtensions.assign(supportedExtensions.cbegin(), supportedExtensions.cend());
}

const std::vector<std::string>& ResourceDecoder::getSupportedExtensions() const
{
    return m_supportedExtensions;
}

} //namespace resource
} //namespace v3d
