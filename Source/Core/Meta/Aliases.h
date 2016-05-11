#pragma once

#include <stdint.h>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef i8 byte;
typedef u8 ubyte;
//----------------------------------------------------------------------------
//typedef i16 short;
typedef u16 ushort;
//----------------------------------------------------------------------------
typedef i32 word;
typedef u32 uword;
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
decltype(std::declval<_Lhs>()+std::declval<_Rhs>()) Max(_Lhs lhs, _Rhs rhs) { return lhs < rhs ? rhs : lhs; }
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
decltype(std::declval<_Lhs>()+std::declval<_Rhs>()) Min(_Lhs lhs, _Rhs rhs) { return lhs < rhs ? lhs : rhs; }
//----------------------------------------------------------------------------
typedef struct uint128_t {
    u64 lo, hi;

    friend bool operator ==(const uint128_t& lhs, const uint128_t& rhs) { return lhs.hi == rhs.hi && lhs.lo == rhs.lo; }
    friend bool operator !=(const uint128_t& lhs, const uint128_t& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator < (const uint128_t& lhs, const uint128_t& rhs) { return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi; }
    friend bool operator >=(const uint128_t& lhs, const uint128_t& rhs) { return !operator < (lhs, rhs); }

    friend void swap(uint128_t& lhs, uint128_t& rhs) { std::swap(lhs.lo, rhs.lo); std::swap(lhs.hi, rhs.hi); }

    template <typename _Char, typename _Traits>
    friend std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const uint128_t& v) {
        const _Char f = oss.fill();
        const std::streamsize w = oss.width();
        return oss << std::hex << std::setfill('0') << std::setw(16) << v.hi << v.lo << std::dec << std::setfill(f) << std::setw(w);
    }
}   u128;
//----------------------------------------------------------------------------
typedef struct uint256_t {
    uint128_t lo, hi;

    friend bool operator ==(const uint256_t& lhs, const uint256_t& rhs) { return lhs.hi == rhs.hi && lhs.lo == rhs.lo; }
    friend bool operator !=(const uint256_t& lhs, const uint256_t& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator < (const uint256_t& lhs, const uint256_t& rhs) { return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi; }
    friend bool operator >=(const uint256_t& lhs, const uint256_t& rhs) { return !operator < (lhs, rhs); }

    friend void swap(uint256_t& lhs, uint256_t& rhs) { std::swap(lhs.lo, rhs.lo); std::swap(lhs.hi, rhs.hi); }

    template <typename _Char, typename _Traits>
    friend std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const uint256_t& v) {
        return oss << hi << lo;
    }
}   u256;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
