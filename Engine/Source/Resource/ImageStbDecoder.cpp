#include "ImageStbDecoder.h"

//#ifdef USE_STB
#define STB_IMAGE_IMPLEMENTATION

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
//#define STBI_ONLY_PSD
//#define STBI_ONLY_GIF
//#define STBI_ONLY_HDR
//#define STBI_ONLY_PIC

#define STBI_NO_STDIO

#include "ThirdParty/stb/stb_image.h"

namespace v3d
{
namespace resource
{

ImageStbDecoder::ImageStbDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader & header) noexcept
    : ResourceDecoder(supportedExtensions)
{
}

ImageStbDecoder::~ImageStbDecoder()
{
}

Resource * ImageStbDecoder::decode(const stream::Stream * stream, const std::string & name)
{
    if (stream->size() > 0)
    {
//#ifdef USE_STB
        stream->seekBeg(0);
        u8* source = stream->map(stream->size());

        s32 sizeX;
        s32 sizeY;
        s32 componentCount = 0;
        s32 req_componentCount = 0;

        u8* data = stbi_load_from_memory(source, stream->size(), &sizeX, &sizeY, &componentCount, req_componentCount);

        //TODO
        int a = 0;
    }

    return nullptr;
}

} // namespace resource
} // namespace v3d
