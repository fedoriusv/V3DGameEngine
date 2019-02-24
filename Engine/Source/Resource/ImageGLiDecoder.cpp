#include "ImageGLiDecoder.h"
#include "Stream/StreamManager.h"
#include "Resource/Image.h"

#include "Utils/Logger.h"

#if USE_GLI
#   include <gli/gli.hpp>
#endif //USE_GLI

namespace v3d
{
namespace resource
{

ImageGLiDecoder::ImageGLiDecoder(std::vector<std::string> supportedExtensions, const resource::ImageHeader & header, bool readHeader) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_header(header)
    , m_readHeader(readHeader)
{
}

ImageGLiDecoder::~ImageGLiDecoder()
{
}

Resource * ImageGLiDecoder::decode(const stream::Stream * stream, const std::string & name)
{
    if (stream->size() > 0)
    {
#ifdef USE_GLI
        stream->seekBeg(0);
        u8* source = stream->map(stream->size());

        gli::texture texture = gli::load(reinterpret_cast<c8*>(source), stream->size());
        stream->unmap();

        if (texture.empty())
        {
            LOG_ERROR("ImageGLiDecoder::decode fail load file %s", name.c_str());
            ASSERT(false, "fail");
            return nullptr;
        }

        bool flipY = true;
        if (m_readHeader)
        {
            flipY = m_header._flipY;
        }

        if (flipY)
        {
            texture = gli::flip(texture);
            ASSERT(!texture.empty(), "fail");
        }

        auto covertFormat = [](gli::format format) -> renderer::Format
        {
            switch (format)
            {
            case gli::format::FORMAT_RGB_DXT1_UNORM_BLOCK8:
                return renderer::Format::Format_BC1_RGB_UNorm_Block;

            case gli::format::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
                return renderer::Format::Format_BC1_RGBA_UNorm_Block;

            case gli::format::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
                return renderer::Format::Format_BC2_UNorm_Block;

            case gli::format::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
                return renderer::Format::Format_BC3_UNorm_Block;

            case gli::format::FORMAT_UNDEFINED:
            default:
                ASSERT(false, "not found");
            }

            return renderer::Format::Format_Undefined;
        };

        resource::ImageHeader* newHeader = new resource::ImageHeader(m_header);
        newHeader->_dimension.width = static_cast<u32>(texture.extent().x);
        newHeader->_dimension.height = static_cast<u32>(texture.extent().y);
        newHeader->_dimension.depth = static_cast<u32>(texture.extent().z);;
        newHeader->_layers = static_cast<u32>(texture.layers());
        newHeader->_mips = static_cast<u32>(texture.levels());
        newHeader->_format = covertFormat(texture.format());
        newHeader->_size = texture.size();
        newHeader->_flipY = flipY;

        stream::Stream* imageStream = stream::StreamManager::createMemoryStream(texture.data(), static_cast<u32>(newHeader->_size));

        resource::Image* image = new resource::Image(newHeader);
        image->init(imageStream);

        return image;

#endif //USE_GLI
    }

    return nullptr;
}

} // namespace resource
} // namespace v3d
