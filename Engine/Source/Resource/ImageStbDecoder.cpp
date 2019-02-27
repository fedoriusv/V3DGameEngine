#include "ImageStbDecoder.h"
#include "Stream/StreamManager.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

#if USE_STB
#   define STB_IMAGE_IMPLEMENTATION
#   define STBI_NO_STDIO

#   define STBI_ONLY_JPEG
#   define STBI_ONLY_PNG
#   define STBI_ONLY_BMP
#   define STBI_ONLY_TGA
//#    define STBI_ONLY_PSD
//#    define STBI_ONLY_GIF
//#    define STBI_ONLY_HDR
//#    define STBI_ONLY_PIC

#   include <ThirdParty/stb/stb_image.h>
#endif //USE_STB

namespace v3d
{
namespace resource
{

ImageStbDecoder::ImageStbDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader & header, bool readHeader) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_readHeader(readHeader)
{
}

ImageStbDecoder::~ImageStbDecoder()
{
}

Resource * ImageStbDecoder::decode(const stream::Stream * stream, const std::string & name)
{
    if (stream->size() > 0)
    {
#ifdef USE_STB
        stream->seekBeg(0);
        u8* source = stream->map(stream->size());

        s32 sizeX;
        s32 sizeY;
        s32 componentCount = 0;
        s32 req_componentCount = 0;
        void* data = nullptr;
        renderer::Format format = renderer::Format::Format_Undefined;

        if (m_readHeader)
        {
            if (m_header._flipY)
            {
                stbi_set_flip_vertically_on_load(true);
            }

            req_componentCount = 4;
            //TODO:
        }

#if DEBUG
        utils::Timer timer;
        timer.start();
#endif
        bool is16BitPerChannel = stbi_is_16_bit_from_memory(source, stream->size());
        if (!is16BitPerChannel)
        {
            //8-bit per channel
            stbi_info_from_memory(source, stream->size(), &sizeX, &sizeY, &componentCount);
            if (componentCount == 3)
            {
                req_componentCount = 4; //Render doesn't support RGB8 for PC. Use instead RGBA8
            }
            else
            {
                req_componentCount = componentCount;
            }

            data = stbi_load_from_memory(source, stream->size(), &sizeX, &sizeY, &componentCount, req_componentCount);
            LOG_DEBUG("ImageStbDecoder::decode load image %s, size [%d, %d], components %d", name.c_str(), sizeX, sizeY, componentCount);

            stream->unmap();

            if (!data)
            {
                LOG_ERROR("ImageStbDecoder::decode fail, Error : %s", stbi_failure_reason());
                ASSERT(false, "load failed");
                return nullptr;
            }
            componentCount = req_componentCount;

            auto convert8BitFormat = [](s32 componentCount) -> renderer::Format
            {
                switch (componentCount)
                {
                case 1:
                    return renderer::Format::Format_R8_UNorm;
                case 2:
                    return renderer::Format::Format_R8G8_UNorm;
                case 3:
                    return renderer::Format::Format_R8G8B8_UNorm;
                case 4:
                    return renderer::Format::Format_R8G8B8A8_UNorm;
                default:
                    ASSERT(false, "wrong counter");
                };

                return renderer::Format::Format_R8G8B8A8_UNorm;
            };

            format = convert8BitFormat(componentCount);
        }
        else
        {
            bool isFloatFormat = stbi_is_hdr_from_memory(source, stream->size());
            if (isFloatFormat)
            {
                auto convertFloatFormat = [](s32 componentCount) -> renderer::Format
                {
                    switch (componentCount)
                    {
                    case 1:
                        return renderer::Format::Format_R32_SFloat;
                    case 2:
                        return renderer::Format::Format_R32G32_SFloat;
                    case 3:
                        return renderer::Format::Format_R32G32B32_SFloat;
                    case 4:
                        return renderer::Format::Format_R32G32B32A32_SFloat;
                    default:
                        ASSERT(false, "wrong counter");
                    };

                    return renderer::Format::Format_R16G16B16A16_UNorm;
                };

                data = stbi_loadf_from_memory(source, stream->size(), &sizeX, &sizeY, &componentCount, req_componentCount);
                if (!data)
                {
                    ASSERT(false, "load failed");
                    return nullptr;
                }

                format = convertFloatFormat(componentCount);
            }
            else
            {
                auto convert16BitFormat = [](s32 componentCount) -> renderer::Format
                {
                    switch (componentCount)
                    {
                    case 1:
                        return renderer::Format::Format_R16_UNorm;
                    case 2:
                        return renderer::Format::Format_R16G16_UNorm;
                    case 3:
                        return renderer::Format::Format_R16G16B16_UNorm;
                    case 4:
                        return renderer::Format::Format_R16G16B16A16_UNorm;
                    default:
                        ASSERT(false, "wrong counter");
                    };

                    return renderer::Format::Format_R16G16B16A16_UNorm;
                };

                data = stbi_load_16_from_memory(source, stream->size(), &sizeX, &sizeY, &componentCount, req_componentCount);
                if (!data)
                {
                    ASSERT(false, "load failed");
                    return nullptr;
                }

                format = convert16BitFormat(componentCount);
            }
        }

#if DEBUG
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("ImageStbDecoder::decode , image %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif

        resource::ImageHeader* newHeader = new resource::ImageHeader(m_header);
        newHeader->_dimension.width = static_cast<u32>(sizeX);
        newHeader->_dimension.height = static_cast<u32>(sizeY);
        newHeader->_dimension.depth = 1;
        newHeader->_format = format;
        newHeader->_size = sizeX * sizeY * req_componentCount;
        newHeader->_flipY = false;

        stream::Stream* imageStream = stream::StreamManager::createMemoryStream(data, static_cast<u32>(newHeader->_size));

        resource::Image* image = new resource::Image(newHeader);
        image->init(imageStream);

        stbi_image_free(data);

        return image;

#endif //USE_STB
    }

    return nullptr;
}

} // namespace resource
} // namespace v3d
