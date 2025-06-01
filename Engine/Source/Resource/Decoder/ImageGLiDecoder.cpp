#include "ImageGLiDecoder.h"

#include "Stream/StreamManager.h"
#include "Resource/Bitmap.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Resource/Loader/ImageFileLoader.h"

#ifdef USE_GLI
#   include "gli/gli/gli.hpp" //Make sure glm is used from own folder
#   include "gli/gli/generate_mipmaps.hpp"

#define LOG_LOADIMG_TIME (DEBUG || 1)

namespace v3d
{
namespace resource
{

ImageGLiDecoder::ImageGLiDecoder(const std::vector<std::string>& supportedExtensions) noexcept
    : ImageDecoder(supportedExtensions)
{
}

ImageGLiDecoder::ImageGLiDecoder(std::vector<std::string>&& supportedExtensions) noexcept
    : ImageDecoder(supportedExtensions)
{
}

ImageGLiDecoder::~ImageGLiDecoder()
{
}

Resource* ImageGLiDecoder::decode(const stream::Stream* stream, const Policy* policy, u32 flags, const std::string& name) const
{
    if (stream->size() > 0)
    {
        stream->seekBeg(0);
        u8* source = stream->map(stream->size());

#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif //LOG_LOADIMG_TIME

        gli::texture texture = gli::load(reinterpret_cast<c8*>(source), stream->size());
        stream->unmap();

        if (texture.empty())
        {
            LOG_ERROR("ImageGLiDecoder::decode fail load file %s", name.c_str());
            ASSERT(false, "fail");
            return nullptr;
        }

        if (flags & ImageLoaderFlag::ImageLoader_FlipY)
        {
            texture = gli::flip(texture);
            ASSERT(!texture.empty(), "fail");
        }

        if ((flags & ImageLoaderFlag::ImageLoader_GenerateMipmaps) && texture.max_level() == 0)
        {
            if (gli::is_compressed(texture.format()))
            {
                LOG_WARNING("ImageGLiDecoder::decode. Can't generate mipmaps, image has compressed format");
                ASSERT(false, "error");
            }
            else
            {
                //TODO: check
                //texture = gli::generate_mipmaps(texture, gli::filter::FILTER_LINEAR);
                NOT_IMPL;
            }
        }

        auto covertFormat = [](gli::format format) -> renderer::Format
        {
            switch (format)
            {
            case gli::format::FORMAT_RGBA8_UNORM_PACK8:
                return renderer::Format::Format_R8G8B8A8_UNorm;

            case gli::format::FORMAT_RGBA8_SRGB_PACK8:
                return renderer::Format::Format_R8G8B8A8_SRGB;

            case gli::format::FORMAT_BGRA8_UNORM_PACK8:
                return renderer::Format::Format_B8G8R8A8_UNorm;

            case gli::format::FORMAT_BGRA8_SRGB_PACK8:
                return renderer::Format::Format_B8G8R8A8_SRGB;

            case gli::format::FORMAT_RGB_DXT1_UNORM_BLOCK8:
                return renderer::Format::Format_BC1_RGB_UNorm_Block;

            case gli::format::FORMAT_RGB_DXT1_SRGB_BLOCK8:
                return renderer::Format::Format_BC1_RGB_SRGB_Block;

            case gli::format::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
                return renderer::Format::Format_BC1_RGBA_UNorm_Block;

            case gli::format::FORMAT_RGBA_DXT1_SRGB_BLOCK8:
                return renderer::Format::Format_BC1_RGBA_SRGB_Block;

            case gli::format::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
                return renderer::Format::Format_BC2_UNorm_Block;

            case gli::format::FORMAT_RGBA_DXT3_SRGB_BLOCK16:
                return renderer::Format::Format_BC2_SRGB_Block;

            case gli::format::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
                return renderer::Format::Format_BC3_UNorm_Block;

            case gli::format::FORMAT_RGBA_DXT5_SRGB_BLOCK16:
                return renderer::Format::Format_BC3_SRGB_Block;

            case gli::format::FORMAT_R_ATI1N_UNORM_BLOCK8:
                return renderer::Format::Format_BC4_UNorm_Block;

            case gli::format::FORMAT_R_ATI1N_SNORM_BLOCK8:
                return renderer::Format::Format_BC4_SNorm_Block;

            case gli::format::FORMAT_RG_ATI2N_UNORM_BLOCK16:
                return renderer::Format::Format_BC5_UNorm_Block;

            case gli::format::FORMAT_RG_ATI2N_SNORM_BLOCK16:
                return renderer::Format::Format_BC5_SNorm_Block;

            case gli::format::FORMAT_RGB_BP_UFLOAT_BLOCK16:
                return renderer::Format::Format_BC6H_UFloat_Block;

            case gli::format::FORMAT_RGB_BP_SFLOAT_BLOCK16:
                return renderer::Format::Format_BC6H_SFloat_Block;

            case gli::format::FORMAT_RGBA_BP_UNORM_BLOCK16:
                return renderer::Format::Format_BC7_UNorm_Block;

            case gli::format::FORMAT_RGBA_BP_SRGB_BLOCK16:
                return renderer::Format::Format_BC7_SRGB_Block;


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

        u32 imageStreamSize = static_cast<u32>(texture.size()) + sizeof(u32);
        stream::Stream* imageStream = stream::StreamManager::createMemoryStream(nullptr, imageStreamSize);
        ASSERT(imageStream, "nullptr");

        math::Dimension3D dimension(static_cast<u32>(texture.extent().x), static_cast<u32>(texture.extent().y), static_cast<u32>(texture.extent().z));
        renderer::Format format = covertFormat(texture.format());
        imageStream->write<math::Dimension3D>(dimension);
        imageStream->write<renderer::Format>(format);
        imageStream->write<u32>(static_cast<u32>(texture.layers()));
        imageStream->write<u32>(static_cast<u32>(texture.levels()));
        imageStream->write<u32>(static_cast<u32>(texture.size()));
        imageStream->write(texture.data(), static_cast<u32>(texture.size()));

        Bitmap::BitmapHeader header;
        resource::ResourceHeader::fill(&header, name, imageStream->size(), 0);

        resource::Resource* image = V3D_NEW(Bitmap, memory::MemoryLabel::MemoryObject)(header);
        if (!image->load(imageStream))
        {
            LOG_ERROR("ImageGLiDecoder::decode: load is falied, %s", name.c_str());

            V3D_DELETE(image, memory::MemoryLabel::MemoryObject);
            image = nullptr;
        }
        stream::StreamManager::destroyStream(imageStream);

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("ImageGLiDecoder::decode:, the image %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

        return image;
    }

    return nullptr;
}

} // namespace resource
} // namespace v3d
#endif //USE_GLI
