#pragma once

#include <type_traits>

namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef unsigned char       u8;
    typedef signed char         s8;
    typedef char                c8;
    typedef wchar_t             w16;
    typedef unsigned short      u16;
    typedef signed short        s16;
    typedef unsigned int        u32;
    typedef signed int          s32;
    typedef unsigned long long  u64;
    typedef signed long long    s64;
    typedef float               f32;
    typedef double              f64;
    typedef long double         f80;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename E>
    constexpr auto toEnumType(E enumerator) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(enumerator);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    concept FloatType = std::is_same_v<T, f32> || std::is_same_v<T, f64> || std::is_same_v<T, f80>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    concept UIntType = std::is_same_v<T, u32> || std::is_same_v<T, u64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d