#pragma once

#include <math.h>
#include "Types.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr f32 k_tolerance32 = 0.00005f;
    constexpr f64 k_tolerance64 = 0.000005;

    constexpr f32 k_pi       = 3.14159265359f;
    constexpr f32 k_rcpPi    = 1.0f / k_pi;
    constexpr f32 k_pi2      = k_pi / 2.0f;
    constexpr f32 k_2pi      = k_pi * 2.0f;
    constexpr f32 k_degToRad = k_pi / 180.0f;
    constexpr f32 k_radToDeg = 180.0f / k_pi;

    constexpr f32 k_cos1  = 0.99984769515f;
    constexpr f32 k_cos5  = 0.99619469809f;
    constexpr f32 k_cos10 = 0.98480775301f;
    constexpr f32 k_cos15 = 0.96592582628f;
    constexpr f32 k_cos30 = 0.86602540378f;
    constexpr f32 k_cos45 = 0.70710678118f;
    constexpr f32 k_cos60 = 0.5f;
    constexpr f32 k_cos75 = 0.25881904510f;
    constexpr f32 k_cos80 = 0.17364817766f;
    constexpr f32 k_cos85 = 0.08715574274f;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    [[nodiscard]] const T& min(const T& a, const T& b);
    template<class T>
    [[nodiscard]] const T& min(const T& a, const T& b, const T& c);

    template<class T>
    [[nodiscard]] const T& max(const T& a, const T& b);
    template<class T>
    [[nodiscard]] const T& max(const T& a, const T& b, const T& c);

    template<class T>
    [[nodiscard]] T abs(const T& a);

    template<class T>
    [[nodiscard]] T sign(const T& a);

    template <class T>
    [[nodiscard]] const T& clamp(const T& value, const T& low, const T& high);

    [[nodiscard]] bool isEquals(f32 a, f32 b, f32 tolerance = k_tolerance32);
    [[nodiscard]] bool isEquals(s32 a, s32 b, s32 tolerance = 0);
    [[nodiscard]] bool isEquals(u32 a, u32 b, u32 tolerance = 0);

    [[nodiscard]] bool isZero(f64 a, f64 tolerance = k_tolerance64);
    [[nodiscard]] bool isZero(f32 a, f32 tolerance = k_tolerance32);
    [[nodiscard]] bool isZero(s32 a, s32 tolerance = 0);
    [[nodiscard]] bool isZero(u32 a, u32 tolerance = 0);

    [[nodiscard]] bool isPowerOf2(u32 a);
    [[nodiscard]] u32 getSmallestPowerOf2(u32 val);

    [[nodiscard]] int getShift(u32 val);
    [[nodiscard]] u32 getBitCount(u32 val);
    [[nodiscard]] void setbit_cond(u32& state, s32 condition, u32 mask);

    [[nodiscard]] f32 round(f32 x);
    [[nodiscard]] f64 round(f64 x);
    template<class T>
    [[nodiscard]] T round(T x, s32 decimalPlaces);

    template<class T>
    [[nodiscard]] T alignUp(T val, T alignment);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Math.inl"
