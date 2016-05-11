#pragma once

#include <intrin.h>
#include <limits.h>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct BitCount {
    enum : size_t { value = (sizeof(T) << 3) };

    static constexpr size_t Words(size_t bits) {
        return (bits+value-1)/value;
    }

    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    static size_t Set(T v) {
        v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
        v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
        v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
        return (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT; // count
    }
};
//----------------------------------------------------------------------------
template <typename T>
typename std::enable_if<std::is_integral<T>::value, size_t>::type CountBitsSet(T value) {
    return BitCount<T>::Set(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if defined(CPP_VISUALSTUDIO)
inline size_t FloorLog2(size_t value) {
    unsigned long log2;
#if defined(ARCH_X64)
    _BitScanReverse64(&log2, value);
#elif defined(ARCH_X86)
    _BitScanReverse(&log2, value);
#else
#   error "unsupported architecture !"
#endif
    return log2;
}
#elif defined(CPP_GCC) or defined(CPP_CLANG)
// TODO : http://www.boost.org/doc/libs/master/boost/intrusive/detail/math.hpp
#   error "not implemented !"
#else
// http://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
inline u32 FloorLog2(u32 value) {
    const u32 sMultiplyDeBruijnBitPosition32[32] = {
         0,  9,  1, 10, 13, 21,  2, 29,
        11, 14, 16, 18, 22, 25,  3, 30,
         8, 12, 20, 28, 15, 17, 24,  7,
        19, 27, 23,  6, 26,  5,  4, 31
    };

    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;

    return sMultiplyDeBruijnBitPosition32[(u32)(value*0x07C4ACDDul) >> 27];
}
inline u64 FloorLog2(u64 value) {
    const u64 sMultiplyDeBruijnBitPosition64[64] = {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5
    };

    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;

    return sMultiplyDeBruijnBitPosition64[((u64)((value - (value >> 1))*0x07EDD5E59A4E28C2ull)) >> 58];
}
#endif
//----------------------------------------------------------------------------
inline size_t CeilLog2(size_t value) {
    return static_cast<size_t>(!IS_POW2(value)) + FloorLog2(value);
}
//----------------------------------------------------------------------------
inline size_t HighestBitSet(size_t value) {
    return CeilLog2(value);
}
//----------------------------------------------------------------------------
inline size_t NextPow2(size_t value) {
    return size_t(1u) << CeilLog2(value);
}
//----------------------------------------------------------------------------
inline size_t PrevOrEqualPow2(size_t value) {
    return size_t(1u) << FloorLog2(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
