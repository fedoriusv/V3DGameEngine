#pragma once

#include "Common.h"
#include "FNV-1a.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <u32 SIZE>
    class ConstexprString
    {
    public:

        constexpr ConstexprString(const c8(&str)[SIZE]) noexcept
        {
            for (u32 i = 0; i < SIZE; ++i) {
                m_str[i] = str[i];
            }
        }

        constexpr const c8* c_str() const
        {
            return m_str;
        }

        constexpr u32 size() const
        {
            return SIZE - 1;
        }

        constexpr char operator[](c8 index) const
        {
            return index < SIZE ? m_str[index] : '\0';
        }

    private:

        c8 m_str[SIZE];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief StringID. Compile time string.
    * View to exernal memory. Keep data alive until TStringID exists
    * Does not keep internal copy of memory.
    * Data must be immutable or hash should be recalculated. Call recalculateHash
    */
    template <UIntType T>
    class V3D_API TStringID
    {
    public:

        constexpr TStringID(T hash, const c8* str) noexcept
            : m_hash(hash)
            , m_str(str)
        {
        }

        template <u32 N>
        constexpr TStringID(const c8(&str)[N]) noexcept
            : m_hash(fnv1a_hash<T>(str))
            , m_str(str)
        {
        }

        constexpr TStringID() noexcept
            : m_hash(0)
            , m_str("")
        {
        }

        constexpr TStringID(const TStringID& other) noexcept
            : m_hash(other.m_hash)
            , m_str(other.m_str)
        {
        }

        constexpr T value() const
        {
            return m_hash;
        }

        constexpr std::string_view name() const
        { 
            return m_str;
        }

        constexpr bool operator==(const TStringID<T>& other) const
        {
            return m_hash == other.m_hash;
        }

        constexpr bool operator<(const TStringID<T>& other) const
        {
            return m_hash < other.m_hash;
        }

        constexpr void recalculateHash()
        {
            m_hash = fnv1a_hash<T>(m_str);
        }

    private:
        T         m_hash;
        const c8* m_str;
    };

    template <UIntType T = u64, u32 N>
    consteval TStringID<T> MakeStringID(const c8(&str)[N])
    {
        return TStringID<T>(fnv1a_hash<T>(str), str);
    }

    template <UIntType T = u64>
    constexpr TStringID<T> MakeRuntimeStringID(const std::string& str)
    {
        return TStringID<T>(fnv1a_hash<T>(str.data()), str.data());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using StringID = TStringID<u64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d