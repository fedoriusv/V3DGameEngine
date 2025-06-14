namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    inline T sign(const T& a)
    {
        if (a == (T)0)
        {
            return (T)0;
        }
        return a < (T)0 ? (T)-1 : (T)1;
    }

    inline bool isEquals(f32 a, f32 b, f32 tolerance)
    {
        return (a + tolerance >= b) && (a - tolerance <= b);
    }

    inline bool isEquals(s32 a, s32 b, s32 tolerance)
    {
        return (a + tolerance >= b) && (a - tolerance <= b);
    }

    inline bool isEquals(u32 a, u32 b, u32 tolerance)
    {
        return (a + tolerance >= b) && (a - tolerance <= b);
    }

    inline bool isZero(f64 a, f64 tolerance)
    {
        return std::fabs(a) <= tolerance;
    }

    inline bool isZero(f32 a, f32 tolerance)
    {
        return std::fabsf(a) <= tolerance;
    }

    inline bool isZero(s32 a, s32 tolerance)
    {
        return (a & 0x7ffffff) <= tolerance;
    }

    inline bool isZero(u32 a, u32 tolerance)
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

    inline f32 round(f32 x)
    {
        return std::floorf(x + 0.5f);
    }

    inline f64 round(f64 x)
    {
        return std::floor(x + 0.5);
    }

    template<class T>
    inline T round(T x, s32 decimalPlaces)
    {
        s32 ix = (s32)x;
        T frac = x - (T)ix;

        T p = std::pow(10.f, decimalPlaces);
        T f = (T)((s32)round(frac * p));
        f /= p;

        return (T)ix + f;
    }

    template<class T>
    inline T alignUp(T val, T alignment)
    {
        return ((val) + (alignment)-1) & ~((alignment) - 1);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d