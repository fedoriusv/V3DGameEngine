#include "ImageGLiDecoder.h"
#include "Stream/StreamManager.h"
#include "Resource/Image.h"

#include "Utils/Logger.h"
#include "Utils/Timer.h"

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

#if DEBUG
        utils::Timer timer;
        timer.start();
#endif
        gli::texture texture = gli::load(reinterpret_cast<c8*>(source), stream->size());
        stream->unmap();

        if (texture.empty())
        {
            LOG_ERROR("ImageGLiDecoder::decode fail load file %s", name.c_str());
            ASSERT(false, "fail");
            return nullptr;
        }

        bool flipY = gli::is_s3tc_compressed(texture.format());
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


            case gli::format::FORMAT_RGB_ETC2_UNORM_BLOCK8:
                return renderer::Format::Format_ETC2_R8G8B8_UNorm_Block;

            case gli::format::FORMAT_RGB_ETC2_SRGB_BLOCK8:
                return renderer::Format::Format_ETC2_R8G8B8_SRGB_Block;

            case gli::format::FORMAT_RGBA_ETC2_UNORM_BLOCK8:
                return renderer::Format::Format_ETC2_R8G8B8A1_UNorm_Block;

            case gli::format::FORMAT_RGBA_ETC2_SRGB_BLOCK8:
                return renderer::Format::Format_ETC2_R8G8B8A1_SRGB_Block;

            case gli::format::FORMAT_RGBA_ETC2_UNORM_BLOCK16:
                return renderer::Format::Format_ETC2_R8G8B8A8_UNorm_Block;

            case gli::format::FORMAT_RGBA_ETC2_SRGB_BLOCK16:
                return renderer::Format::Format_ETC2_R8G8B8A8_SRGB_Block;

            case gli::format::FORMAT_R_EAC_UNORM_BLOCK8:
                return renderer::Format::Format_EAC_R11_UNorm_Block;

            case gli::format::FORMAT_R_EAC_SNORM_BLOCK8:
                return renderer::Format::Format_EAC_R11_SNorm_Block;

            case gli::format::FORMAT_RG_EAC_UNORM_BLOCK16:
                return renderer::Format::Format_EAC_R11G11_UNorm_Block;

            case gli::format::FORMAT_RG_EAC_SNORM_BLOCK16:
                return renderer::Format::Format_EAC_R11G11_SNorm_Block;


            case gli::format::FORMAT_RGBA_ASTC_4X4_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_4x4_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_4X4_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_4x4_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_5X4_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_5x4_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_5X4_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_5x4_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_5X5_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_5x5_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_5X5_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_5x5_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_6X5_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_6x5_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_6X5_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_6x5_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_6X6_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_6x6_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_6X6_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_6x6_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_8X5_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_8x5_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_8X5_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_8x5_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_8X6_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_8x6_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_8X6_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_8x6_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_8x8_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_8X8_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_8x8_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_10X5_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_10x5_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_10X5_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_10x5_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_10X6_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_10x6_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_10X6_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_10x6_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_10X8_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_10x8_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_10X8_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_10x8_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_10X10_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_10x10_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_10X10_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_10x10_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_12X10_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_12x10_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_12X10_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_12x10_SRGB_Block;

            case gli::format::FORMAT_RGBA_ASTC_12X12_UNORM_BLOCK16:
                return renderer::Format::Format_ASTC_12x12_UNorm_Block;

            case gli::format::FORMAT_RGBA_ASTC_12X12_SRGB_BLOCK16:
                return renderer::Format::Format_ASTC_12x12_SRGB_Block;

            case gli::format::FORMAT_UNDEFINED:
            default:
                ASSERT(false, "format not found");
            }

            return renderer::Format::Format_Undefined;
        };

#if DEBUG
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("ImageGLiDecoder::decode , image %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif

        resource::ImageHeader* newHeader = new resource::ImageHeader(m_header);
        newHeader->_dimension.width = static_cast<u32>(texture.extent().x);
        newHeader->_dimension.height = static_cast<u32>(texture.extent().y);
        newHeader->_dimension.depth = static_cast<u32>(texture.extent().z);;
        newHeader->_layers = static_cast<u32>(texture.layers());
        newHeader->_mips = static_cast<u32>(texture.levels());
        newHeader->_format = covertFormat(texture.format());
        newHeader->_size = texture.size();
        newHeader->_flipY = flipY;
#if DEBUG
        newHeader->_debugName = name;
#endif
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
