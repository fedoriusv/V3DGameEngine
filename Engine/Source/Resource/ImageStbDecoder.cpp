#include "ImageStbDecoder.h"

namespace v3d
{
namespace resource
{

ImageStbDecoder::ImageStbDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader & header) noexcept
{
}

ImageStbDecoder::~ImageStbDecoder()
{
}

Resource * ImageStbDecoder::decode(const stream::Stream * stream, const std::string & name)
{
    return nullptr;
}

} // namespace resource
} // namespace v3d
