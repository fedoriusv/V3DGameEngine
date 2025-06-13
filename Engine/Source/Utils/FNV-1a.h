#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    // 32-bit FNV-1a constants
    constexpr u32 k_fnv1a_offset = 0x811C9DC5;
    constexpr u32 k_fnv1a_prime = 0x01000193;

    constexpr u32 fnv1a_hash32(const c8* str, u32 hash = k_fnv1a_offset)
    {
        return (*str) ? fnv1a_hash32(str + 1, (hash ^ static_cast<u8>(*str)) * k_fnv1a_prime) : hash;
    }

    // 64-bit FNV-1a constants
    constexpr u64 k_fnv1a_offset_64 = 0xcbf29ce484222325;
    constexpr u64 k_fnv1a_prime_64 = 0x100000001b3;

    constexpr u64 fnv1a_hash64(const c8* str, u64 hash = k_fnv1a_offset_64)
    {
        return (*str) ? fnv1a_hash64(str + 1, (hash ^ static_cast<u8>(*str)) * k_fnv1a_prime_64) : hash;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<UIntType T>
    constexpr T fnv1a_hash(const c8* str)
    {
        if constexpr (std::is_same_v<T, u64>)
        {
            return fnv1a_hash64(str);
        }
        else if  constexpr (std::is_same_v<T, u32>)
        {
            return fnv1a_hash32(str);
        }

        return 0;
    }

    //template<UIntType T>
    //constexpr T fnv1a_hash(const std::string& str)
    //{
    //    return fnv1a_hash(str.c_str());
    //}

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d