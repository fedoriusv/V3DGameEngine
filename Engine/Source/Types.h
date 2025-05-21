#pragma once

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

    class Type;
    using TypePtr = Type const*;

    template<typename T>
    struct TypeOf
    {
        static TypePtr get()
        {
            return TypeOf<T>::get();
        }
    };

    template<typename T>
    TypePtr type_of()
    {
        return TypeOf<T>::get();
    };

    struct MetaObject
    {
        template<typename T>
        MetaObject(T* obj)
            : _type(type_of<T>())
            , _object(obj)
        {
        }

        MetaObject(TypePtr type)
            : _type(type)
            , _object(nullptr)
        {
        }

        TypePtr _type;
        void*   _object;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d