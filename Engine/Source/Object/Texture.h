#pragma once

#include "Common.h"
#include "Object.h"
#include "CommandList.h"

namespace v3d
{
namespace renderer
{
    class Image;
} //namespace renderer

  /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class TextureTarget : s16
    {
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCubeMap,
        TextureCubeMapArray,
    };

    enum TextureFilter : s16
    {
        TextureFilterNearest = 0x1,
        TextureFilterLinear = 0x2,

        TextureFilterNearestMipmapNearest = 0x4,
        TextureFilterNearestMipmapLinear = 0x8,
        TextureFilterLinearMipmapNearest = 0x16,
        TextureFilterLinearMipmapLinear = 0x32
    };

    enum class TextureAnisotropic
    {
        TextureAnisotropicNone = 0,
        TextureAnisotropic2x = 1 << 1,
        TextureAnisotropic4x = 1 << 2,
        TextureAnisotropic8x = 1 << 3,
        TextureAnisotropic16x = 1 << 4,
    };

    enum class TextureWrap : s16
    {
        TextureRepeat,
        TextureMirroredRepeat,
        TextureClampToEdge,
        TextureClampToBorder,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Texture2D : public Object //ref couter
    {
    public:

        ~Texture2D();

        TextureTarget               getTarget() const;
        TextureFilter               getMinFilter() const;
        TextureFilter               getMagFilter() const;
        TextureWrap                 getWrap() const;
        TextureAnisotropic          getAnisotropic() const;
        u32                         getMipmaps() const;
        const core::Dimension2D&    getSize() const;
        //ImageFormat                 getFormat() const;
        //ImageType                   getType() const;

        void update(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipmapCount, const void* data);
        void read(const core::Dimension2D& offset, const core::Dimension2D& size, u32 mipLevel, void* const data);

    private:

        Texture2D(renderer::CommandList& cmdList,  /*ImageFormat format, ImageType type,*/ const core::Dimension2D& size, u32 mipmapCount, const void* data);

        void deletionNotify() { m_image = nullptr; };

        const TextureTarget         m_target;
        //const ImageFormat              m_format;
        //const ImageType                m_type;
        const u32                   m_mipmapLevel;
        const core::Dimension2D     m_size;

        const s16                   m_filter;
        const TextureAnisotropic    m_anisotropicLevel;
        const TextureWrap           m_wrap;

        renderer::Image*            m_image;

        renderer::CommandList&      m_cmdList;
    };


    class SwapchainTexture : public Object
    {
    public:

        SwapchainTexture() {};
        ~SwapchainTexture() {};

    private:

        renderer::Image* m_image;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
