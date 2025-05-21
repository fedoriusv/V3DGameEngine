#pragma once

#include "Math/Core.h"

namespace v3d
{
namespace color
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct Color8Bit
    {
        constexpr Color8Bit(u8 red, u8 green, u8 blue, u8 alpha = 255) noexcept
            : _red(red)
            , _green(green)
            , _blue(blue)
            , _alpha(alpha)
        {
        }
        
        constexpr Color8Bit(Color8Bit& color) noexcept
            : _red(color._red)
            , _green(color._green)
            , _blue(color._blue)
            , _alpha(color._alpha)
        {
        }

        constexpr Color8Bit(Color8Bit& color, u8 alpha) noexcept
            : _red(color._red)
            , _green(color._green)
            , _blue(color._blue)
            , _alpha(alpha)
        {
        }

        Color8Bit& set(u8 red, u8 green, u8 blue, u8 alpha = 255);

        const Color8Bit operator*(const Color8Bit& color);
        const Color8Bit& operator*=(const Color8Bit& color);

        constexpr operator u32() const
        {
            return getRGBA();
        }

        constexpr u32 getRGBA() const
        {
            return ((static_cast<u32>(_alpha) << 24) | (static_cast<u32>(_blue) << 16) | (static_cast<u32>(_green) << 8) | (static_cast<u32>(_red)));
        }

        constexpr u32 getBGRA() const
        {
            return ((static_cast<u32>(_alpha) << 24) | (static_cast<u32>(_red) << 16) | (static_cast<u32>(_green) << 8) | (static_cast<u32>(_blue)));
        }

        constexpr u32 getARGB() const
        {
            return ((static_cast<u32>(_blue) << 24) | (static_cast<u32>(_green) << 16) | (static_cast<u32>(_red) << 8) | (static_cast<u32>(_alpha)));
        }

        constexpr u32 getABGR() const
        {
            return ((static_cast<u32>(_red) << 24) | (static_cast<u32>(_green) << 16) | (static_cast<u32>(_blue) << 8) | (static_cast<u32>(_alpha)));
        }

        u8 _red;
        u8 _green;
        u8 _blue;
        u8 _alpha;
    };

    inline Color8Bit& Color8Bit::set(u8 red, u8 green, u8 blue, u8 alpha)
    {
        _red = red;
        _green = green;
        _blue = blue;
        _alpha = alpha;
    }

    inline const Color8Bit Color8Bit::operator*(const Color8Bit& color)
    {
        Color8Bit result(*this);
        result *= color;

        return result;
    }

    inline const Color8Bit& Color8Bit::operator*=(const Color8Bit& color)
    {
        _red = static_cast<u8>(math::k_inv255 * _red * color._red + 0.5f);
        _green = static_cast<u8>(math::k_inv255 * _green * color._green + 0.5f);
        _blue = static_cast<u8>(math::k_inv255 * _blue * color._blue + 0.5f);
        _alpha = static_cast<u8>(math::k_inv255 * _alpha * color._alpha + 0.5f);

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr Color8Bit WHITE(255, 255, 255);
    constexpr Color8Bit LTGREY(192, 192, 192);
    constexpr Color8Bit MDGREY(128, 128, 128);
    constexpr Color8Bit DKGREY(64, 64, 64);
    constexpr Color8Bit BLACK(0, 0, 0);
    constexpr Color8Bit RED(255, 0, 0);
    constexpr Color8Bit GREEN(0, 255, 0);
    constexpr Color8Bit BLUE(0, 0, 255);
    constexpr Color8Bit CYAN(0, 255, 255);
    constexpr Color8Bit MAGENTA(255, 0, 255);
    constexpr Color8Bit YELLOW(255, 255, 0);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using ColorRGBF  = math::TVector3D<f32>;
    using ColorRGBAF = math::TVector4D<f32>;
    using ColorRGBA8 = Color8Bit;


    using Color = ColorRGBAF;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace color
} //namespace v3d