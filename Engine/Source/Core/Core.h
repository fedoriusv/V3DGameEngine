#pragma once

namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef unsigned char       u8;
    typedef signed char         s8;
    typedef char                c8;
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
    constexpr auto              toEnumType(E enumerator) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(enumerator);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d

#include "Math.h"
#include "Vector2D.h"
#include "Vector3D.h"
#include "Vector4D.h"
#include "Matrix2D.h"
#include "Matrix3D.h"
#include "Matrix4D.h"
#include "Dimension2D.h"
#include "Dimension3D.h"
#include "Rect.h"