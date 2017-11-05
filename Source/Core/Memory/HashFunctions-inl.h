#pragma once

#include "Core/Memory/HashFunctions.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Understanding the Linux Kernel
// https://books.google.fr/books?id=h0lltXyJ8aIC&lpg=PT109&ots=gO2uM_c7FQ&dq=The%20Magic%20Constant%20linux%20hash&hl=fr&pg=PT109#v=onepage&q=The%20Magic%20Constant%20linux%20hash&f=false
//----------------------------------------------------------------------------
/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001ULL
//----------------------------------------------------------------------------
#ifdef ARCH_X64
#   define CORE_HASH_VALUE_SEED (0xdeadbeefabadcafeULL)
#   define GOLDEN_RATIO_PRIME_SIZE_T GOLDEN_RATIO_PRIME_64
#else
#   define CORE_HASH_VALUE_SEED (0xdeadbeefUL)
#   define GOLDEN_RATIO_PRIME_SIZE_T GOLDEN_RATIO_PRIME_32
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, size_t _Sz = sizeof(T)> struct TPODHash {
    static FORCE_INLINE size_t fn(const T& pod) {
        return hash_mem((const void *)&pod, sizeof(T));
    }
};
template <typename T> struct TPODHash<T, sizeof(u8)> {
    static FORCE_INLINE size_t fn(const T& pod) {
        return hash_uint(size_t(reinterpret_cast<const u8&>(pod)));
    }
};
template <typename T> struct TPODHash<T, sizeof(u16)> {
    static FORCE_INLINE size_t fn(const T& pod) {
        return hash_uint(size_t(reinterpret_cast<const u16&>(pod)));
    }
};
template <typename T> struct TPODHash<T, sizeof(u32)> {
    static FORCE_INLINE size_t fn(const T& pod) {
        return hash_uint(size_t(reinterpret_cast<const u32&>(pod)));
    }
};
template <typename T> struct TPODHash<T, sizeof(u64)> {
    static FORCE_INLINE size_t fn(const T& pod) {
        return hash_uint(reinterpret_cast<const u64&>(pod));
    }
};
template <typename T> struct TPODHash<T, sizeof(u128)> {
    static FORCE_INLINE size_t fn(const T& pod) {
        return hash_uint(reinterpret_cast<const u128&>(pod));
    }
};
template <typename T> struct TPODHash<T, sizeof(u32)*3> {
    struct uint96_t { u64 lo; u32 hi; };
    static FORCE_INLINE size_t fn(const T& pod) {
        const uint96_t& uint96 = reinterpret_cast<const uint96_t&>(pod);
        return hash_uint(u128{ uint96.lo, uint96.hi });
    }
};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename T>
size_t hash_as_pod(const T& pod) {
    return details::TPODHash<T>::fn(pod);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE size_t hash_as_pod_array(const T *ptr, size_t count) {
    return hash_mem(ptr, sizeof(T) * count);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE size_t hash_as_pod_array(const T (&staticArray)[_Dim]) {
    return hash_mem(&staticArray[0], sizeof(T) * _Dim);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _It, typename _Tag>
size_t hash_as_pod_range_impl_(_It&& first, _It&& last, _Tag ) {
    size_t h = CORE_HASH_VALUE_SEED;
    for (; first != last; ++first)
        h = hash_mem(*first, sizeof(*first), h);
    return h;
}
template <typename _It>
size_t hash_as_pod_range_impl_(_It&& first, _It&& last, std::random_access_iterator_tag ) {
    return hash_as_pod_array(&(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename _It>
FORCE_INLINE size_t hash_as_pod_range(_It&& first, _It&& last) {
    typedef typename std::iterator_traits<_It>::iterator_category category;
    return details::hash_as_pod_range_impl_(std::forward<_It>(first), std::forward<_It>(last), category());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}
