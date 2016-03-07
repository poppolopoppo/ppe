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
inline u32 Log2i(u32 v) {
#ifdef _MSC_VER
    unsigned long result;
    ::_BitScanReverse(&result, v);
    return u32(result);
#elif defined(__GNUC__)
    return 31 - __builtin_clz(parNum);
#else
    AssertNotImplemented();
#endif
}
//----------------------------------------------------------------------------
inline u64 Log2i(u64 v) {
#ifdef _MSC_VER
    unsigned long result;
    ::_BitScanReverse64(&result, v);
    return u64(result);
#elif defined(__GNUC__)
    return 63 - __builtin_clz(parNum);
#else
    AssertNotImplemented();
#endif
}
//----------------------------------------------------------------------------
inline size_t CountLeadingZeros(size_t v) { return Log2i(v); }
//----------------------------------------------------------------------------
#if 1
inline size_t RoundToNextHigherPow2_AssumeNotZero(size_t v) {
    return (size_t(1)<<CountLeadingZeros(v-1+v));
}
inline size_t RoundToNextHigherPow2(size_t v) {
    return (0 == v ? 1 : RoundToNextHigherPow2_AssumeNotZero(v));
}
#else
inline u32 RoundToNextHigherPow2(u32 v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
