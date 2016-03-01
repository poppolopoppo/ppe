#pragma once

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
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
