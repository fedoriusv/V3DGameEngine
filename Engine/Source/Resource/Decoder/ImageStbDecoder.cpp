#include "ImageStbDecoder.h"

#include "Stream/StreamManager.h"
#include "Resource/Bitmap.h"
#include "Renderer/Texture.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Resource/Loader/ImageFileLoader.h"

#ifdef USE_STB
#   define STB_IMAGE_IMPLEMENTATION
#   define STB_IMAGE_RESIZE_IMPLEMENTATION
#   define STBI_NO_STDIO

#   define STBI_ONLY_JPEG
#   define STBI_ONLY_PNG
#   define STBI_ONLY_BMP
#   define STBI_ONLY_TGA
#   define STBI_ONLY_HDR
//#    define STBI_ONLY_PSD
//#    define STBI_ONLY_GIF
//#    define STBI_ONLY_PIC

#   include <stb/stb_image.h>
#   include <stb/stb_image_resize.h>

#define LOG_LOADIMG_TIME (DEBUG || 1)

namespace v3d
{
namespace resource
{

static stream::Stream* generateMipMaps(void* baseMipmap, u32 width, u32 height, u32 componentsCount, u32 componentSize, u32 componentType, u32& mipmapsCount)
{
    ASSERT(baseMipmap, "nullptr");

    auto calculateMipSizes = [](u32 width, u32 height, u32 componentCount, u32 sizeBytes, std::vector<std::tuple<u32, u32>>& mipmapsArray) -> u32
        {
            u32 currentWidth = width;
            u32 currentHeight = height;
            u32 countBytes = currentWidth * currentHeight * componentCount * sizeBytes;
            mipmapsArray.push_back({ currentWidth, currentHeight });

            while (currentWidth != 1U && currentHeight != 1U)
            {
                currentWidth = std::max(currentWidth / 2, 1U);
                currentHeight = std::max(currentHeight / 2, 1U);
                countBytes += currentWidth * currentHeight * componentCount * sizeBytes;

                mipmapsArray.push_back({ currentWidth, currentHeight });
            }

            return countBytes;
        };

    std::vector<std::tuple<u32, u32>> mipMapsSizes;
    u32 mipMapsInBytes = calculateMipSizes(width, height, componentsCount, componentSize, mipMapsSizes);
    stream::Stream* dataStream = stream::StreamManager::createMemoryStream(nullptr, mipMapsInBytes);
    ASSERT(dataStream, "nullptr");

    u32 baseMipmapSize = width * height * componentsCount * componentSize;
    dataStream->write(baseMipmap, baseMipmapSize); //copy a base mipmap

    void* srcData = baseMipmap;
    stbir_colorspace colorspace = STBIR_COLORSPACE_LINEAR;
    for (u32 mip = 1; mip < mipMapsSizes.size(); ++mip)
    {
        u32 mipmapSize = std::get<0>(mipMapsSizes[mip]) * std::get<1>(mipMapsSizes[mip]) * componentsCount * componentSize;
        void* dstData = dataStream->map(mipmapSize);

        s32 result = stbir_resize(reinterpret_cast<u8*>(srcData), std::get<0>(mipMapsSizes[mip - 1]), std::get<1>(mipMapsSizes[mip - 1]), 0,
            reinterpret_cast<u8*>(dstData), std::get<0>(mipMapsSizes[mip]), std::get<1>(mipMapsSizes[mip]), 0, (stbir_datatype)componentType, componentsCount,
            -1, 0, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT, colorspace, nullptr);
        ASSERT(result, "mipmap generate failed");

        dataStream->unmap();
        dataStream->seekCur(mipmapSize);

        srcData = dstData;
    }

    mipmapsCount = static_cast<u32>(mipMapsSizes.size());

    dataStream->seekBeg(0);
    return dataStream;
}

template<typename TPolicy>
static stream::Stream* readImage(u8* source, u32 sizeInBytes, const TPolicy& policy, s32& width, s32& height, u32& layers, u32& mipmaps, s32& componentCount, renderer::Format& format)
{
    s32 reqestedComponentCount = 0;
    stream::Stream* dataStream = nullptr;

    if (policy.flipY)
    {
        stbi_set_flip_vertically_on_load(true);
        reqestedComponentCount = 4;
    }

    void* stbData = nullptr;
    bool isFloatFormat = stbi_is_hdr_from_memory(source, sizeInBytes);
    bool is16BitPerChannel = stbi_is_16_bit_from_memory(source, sizeInBytes);
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

        stbData = stbi_loadf_from_memory(source, sizeInBytes, &width, &height, &componentCount, reqestedComponentCount);
        if (!stbData)
        {
            ASSERT(false, "load failed");
            return nullptr;
        }
        format = convertFloatFormat(componentCount);
        ASSERT(!(format == renderer::Format::Format_R32G32B32_SFloat), "fromat is not supported");

        if (policy.generateMipmaps)
        {
            dataStream = generateMipMaps(stbData, width, height, componentCount, sizeof(f32), STBIR_TYPE_FLOAT, mipmaps);
        }
        else
        {
            u32 baseMipmapSize = width * height * componentCount * sizeof(u16);
            dataStream = stream::StreamManager::createMemoryStream(stbData, baseMipmapSize);
        }
    }
    else if (is16BitPerChannel)
    {
        stbi_info_from_memory(source, sizeInBytes, &width, &height, &componentCount);
        if (componentCount == 3)
        {
            reqestedComponentCount = 4; //Render doesn't support RGB16 for PC. Use instead RGBA16
        }
        else
        {
            reqestedComponentCount = componentCount;
        }

        stbData = stbi_load_16_from_memory(source, sizeInBytes, &width, &height, &componentCount, reqestedComponentCount);
        if (!stbData)
        {
            ASSERT(false, "load failed");
            return nullptr;
        }
        componentCount = reqestedComponentCount;

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
        format = convert16BitFormat(componentCount);

        if (policy.generateMipmaps)
        {
            dataStream = generateMipMaps(stbData, width, height, componentCount, sizeof(u16), STBIR_TYPE_UINT16, mipmaps);
        }
        else
        {
            u32 baseMipmapSize = width * height * componentCount * sizeof(u16);
            dataStream = stream::StreamManager::createMemoryStream(stbData, baseMipmapSize);
        }
    }
    else //8-bit per channel
    {
        stbi_info_from_memory(source, sizeInBytes, &width, &height, &componentCount);
        if (componentCount == 3)
        {
            reqestedComponentCount = 4; //Render doesn't support RGB8 for PC. Use instead RGBA8
        }
        else
        {
            reqestedComponentCount = componentCount;
        }

        stbData = stbi_load_from_memory(source, sizeInBytes, &width, &height, &componentCount, reqestedComponentCount);
        if (!stbData)
        {
            LOG_ERROR("ImageStbDecoder::decode fail, Error : %s", stbi_failure_reason());
            ASSERT(false, "load failed");
            return nullptr;
        }
        componentCount = reqestedComponentCount;

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

        if (policy.generateMipmaps)
        {
            dataStream = generateMipMaps(stbData, width, height, componentCount, sizeof(u8), STBIR_TYPE_UINT8, mipmaps);
        }
        else
        {
            u32 baseMipmapSize = width * height * componentCount * sizeof(u8);
            dataStream = stream::StreamManager::createMemoryStream(stbData, baseMipmapSize);
        }
    }

    ASSERT(dataStream, "nullptr");
    stbi_image_free(stbData);

    return dataStream;
}

BitmapStbDecoder::BitmapStbDecoder(const std::vector<std::string>& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
{
}

BitmapStbDecoder::BitmapStbDecoder(std::vector<std::string>&& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
{
}

BitmapStbDecoder::~BitmapStbDecoder()
{
}

Resource* BitmapStbDecoder::decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags, const std::string& name) const
{
    if (stream->size() > 0)
    {
        stream->seekBeg(0);
#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif //LOG_LOADIMG_TIME

        s32 width = 0;
        s32 height = 0;
        u32 layers = 1;
        u32 mipmaps = 1;
        s32 componentCount = 0;
        renderer::Format format = renderer::Format::Format_Undefined;
        u8* source = stream->map(stream->size());
        u32 sizeInBytes = stream->size();
        const BitmapFileLoader::PolicyType& bitmapPolicy = *static_cast<const BitmapFileLoader::PolicyType*>(policy);
        stream::Stream* dataStream = readImage(source, sizeInBytes, bitmapPolicy, width, height, layers, mipmaps, componentCount, format);
        stream->unmap();

        if (!dataStream)
        {
            ASSERT(false, "fail to load");
            return nullptr;
        }
        LOG_DEBUG("BitmapStbDecoder::decode load image %s, size [%d, %d], components %d", name.c_str(), width, height, componentCount);

        u32 imageStreamSize = dataStream->size();
        stream::Stream* imageStream = stream::StreamManager::createMemoryStream(nullptr, imageStreamSize + sizeof(u32));

        math::Dimension3D dimension(static_cast<u32>(width), static_cast<u32>(height), 1);
        format = bitmapPolicy.srgb ? renderer::ImageFormat::convertToSRGB(format) : format;

        imageStream->write<math::Dimension3D>(dimension);
        imageStream->write<renderer::Format>(format);
        imageStream->write<u32>(layers);
        imageStream->write<u32>(mipmaps);
        imageStream->write<u32>(imageStreamSize);
        imageStream->write(dataStream->map(dataStream->size()), dataStream->size());
        dataStream->unmap();

        stream::StreamManager::destroyStream(dataStream);

        Bitmap::BitmapHeader header;
        resource::ResourceHeader::fill(&header, name, imageStream->size(), 0);

        Resource* image = V3D_NEW(Bitmap, memory::MemoryLabel::MemoryObject)(header);
        if (!image->load(imageStream))
        {
            LOG_ERROR("BitmapStbDecoder::decode: load is falied, %s", name.c_str());

            V3D_DELETE(image, memory::MemoryLabel::MemoryObject);
            image = nullptr;
        }
        stream::StreamManager::destroyStream(imageStream);

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("BitmapStbDecoder::decode, the image %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

        return image;
    }

    return nullptr;
}


TextureStbDecoder::TextureStbDecoder(renderer::Device* device, const std::vector<std::string>& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_device(device)
{
}

TextureStbDecoder::TextureStbDecoder(renderer::Device* device, std::vector<std::string>&& supportedExtensions) noexcept
    : ResourceDecoder(supportedExtensions)
    , m_device(device)
{
}

TextureStbDecoder::~TextureStbDecoder()
{
}

Resource* TextureStbDecoder::decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags, const std::string& name) const
{
    if (stream->size() > 0)
    {
        stream->seekBeg(0);
#if LOG_LOADIMG_TIME
        utils::Timer timer;
        timer.start();
#endif //LOG_LOADIMG_TIME

        s32 width = 0;
        s32 height = 0;
        u32 layers = 1;
        u32 mipmaps = 1;
        s32 componentCount = 0;
        renderer::Format format = renderer::Format::Format_Undefined;
        const TextureFileLoader::PolicyType& texturePolicy = *static_cast<const TextureFileLoader::PolicyType*>(policy);
        u8* source = stream->map(stream->size());
        u32 sizeInBytes = stream->size();
        stream::Stream* dataStream = readImage(source, sizeInBytes, texturePolicy, width, height, layers, mipmaps, componentCount, format);
        stream->unmap();

        if (!dataStream)
        {
            ASSERT(false, "fail to load");
            return nullptr;
        }
        LOG_DEBUG("TextureStbDecoder::decode load image %s, size [%d, %d], components %d", name.c_str(), width, height, componentCount);

        u32 imageStreamSize = static_cast<u32>(dataStream->size()) + sizeof(u32);
        stream::Stream* imageStream = stream::StreamManager::createMemoryStream(nullptr, imageStreamSize);
        ASSERT(imageStream, "nullptr");

        renderer::TextureTarget target = renderer::TextureTarget::Texture2D; //2d texture only
        math::Dimension3D dimension(static_cast<u32>(width), static_cast<u32>(height), 1U);
        format = texturePolicy.srgb ? renderer::ImageFormat::convertToSRGB(format) : format;

        imageStream->write<renderer::TextureTarget>(target);
        imageStream->write<math::Dimension3D>(dimension);
        imageStream->write<renderer::Format>(format);
        imageStream->write<renderer::TextureSamples>(renderer::TextureSamples::TextureSamples_x1);
        imageStream->write<u32>(layers);
        imageStream->write<u32>(mipmaps);
        imageStream->write<renderer::TextureUsageFlags>(texturePolicy.usage);
        imageStream->write<u32>(dataStream->size());
        imageStream->write(dataStream->map(dataStream->size()), dataStream->size());
        dataStream->unmap();

        renderer::Texture::TextureHeader header(target);
        resource::ResourceHeader::fill(&header, name, imageStream->size(), 0);

        resource::Resource* image = V3D_NEW(renderer::Texture2D, memory::MemoryLabel::MemoryObject)(m_device, header);
        if (!image->load(imageStream))
        {
            LOG_ERROR("TextureGLiDecoder::decode: load is falied, %s", name.c_str());
            V3D_DELETE(image, memory::MemoryLabel::MemoryObject);
            image = nullptr;
        }
        stream::StreamManager::destroyStream(imageStream);

#if LOG_LOADIMG_TIME
        timer.stop();
        u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
        LOG_DEBUG("TextureStbDecoder::decode, the image %s, is loaded. Time %.4f sec", name.c_str(), static_cast<f32>(time) / 1000.0f);
#endif //LOG_LOADIMG_TIME

        return image;
    }

    return nullptr;
}

} // namespace resource
} // namespace v3d
#endif //USE_STB
