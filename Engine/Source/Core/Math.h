#pragma once

#include <math.h>

namespace v3d
{
namespace core
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
    const T& min(const T& a, const T& b)
    {
        return a < b ? a : b;
    }

    template<class T>
    const T& min(const T& a, const T& b, const T& c)
    {
        return a < b ? min(a, c) : min(b, c);
    }

    template<class T>
    const T& max(const T& a, const T& b)
    {
        return a < b ? b : a;
    }

    template<class T>
    const T& max(const T& a, const T& b, const T& c)
    {
        return a < b ? max(b, c) : max(a, c);
    }

    template<class T>
    T abs(const T& a)
    {
        return a < (T)0 ? -a : a;
    }

    template<class T>
    T sign(const T& a)
    {
        if (a == (T)0)
        {
            return (T)0;
        }
        return a < (T)0 ? (T)-1 : (T)1;
    }

    template <class T>
    const T clamp(const T& value, const T& low, const T& high)
    {
        return min(max(value, low), high);
    }

    inline bool isEquals(f32 a, f32 b, f32 tolerance = k_tolerance32)
    {
        return (a + tolerance >= b) && (a - tolerance <= b);
    }

    inline bool isEquals(s32 a, s32 b, s32 tolerance = 0)
    {
        return (a + tolerance >= b) && (a - tolerance <= b);
    }

    inline bool isEquals(u32 a, u32 b, u32 tolerance = 0)
    {
        return (a + tolerance >= b) && (a - tolerance <= b);
    }

    inline bool isZero(f64 a, f64 tolerance = k_tolerance64)
    {
        return fabs(a) <= tolerance;
    }

    inline bool isZero(f32 a, f32 tolerance = k_tolerance32)
    {
        return fabsf(a) <= tolerance;
    }

    inline bool isZero(s32 a, s32 tolerance = 0)
    {
        return (a & 0x7ffffff) <= tolerance;
    }

    inline bool isZero(u32 a, u32 tolerance = 0)
    {
        return a <= tolerance;
    }

    inline bool isPowerOf2(u32 a)
    {
        return !(a & (a - 1)) && a;
    }

    inline u32 getSmallestPowerOf2(u32 val)
    {
        u32 result = 1;
        while (result < val)
        {
            result <<= 1;
        }
        return result;
    }

    inline int getShift(u32 val)
    {
        int shift = -1;
        while (val)
        {
            shift++;
            val >>= 1;
        }
        return shift;
    }

    inline u32 getBitCount(u32 val)
    {
        u32 count = 0;
        while (val)
        {
            if (val & 0x1)
            {
                count++;
            }
            val >>= 1;
        }
        return count;
    }

    inline void setbit_cond(u32 &state, s32 condition, u32 mask)
    {
        state ^= ((-condition >> 31) ^ state) & mask;
    }

    inline f32 round(f32 x)
    {
        return floorf(x + 0.5f);
    }

    inline f64 round(f64 x)
    {
        return floor(x + 0.5);
    }

    template<class T>
    T round(T x, s32 decimalPlaces)
    {
        s32 ix = (s32)x;
        T frac = x - (T)ix;

        T p = pow(10.f, decimalPlaces);
        T f = (T)((s32)round(frac * p));
        f /= p;

        return (T)ix + f;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace core
} //namespace v3d
