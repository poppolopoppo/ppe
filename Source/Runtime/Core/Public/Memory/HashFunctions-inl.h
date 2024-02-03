#pragma once

#include "Memory/HashFunctions.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
// Any pod size fall back to generic hash_mem()
template <typename T, size_t _Sz = sizeof(T)>
struct TCRC32Hash {
    static FORCE_INLINE size_t fn(const T& pod) {
        return hash_crc32((const void *)&pod, sizeof(T));
    }
};
//----------------------------------------------------------------------------
// Types with a small aligned size are using specialized hash functions
template <typename T>
struct TCRC32Hash<T, sizeof(u8)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_crc32(u32(reinterpret_cast<const u8&>(pod)));
    }
};
template <typename T>
struct TCRC32Hash<T, sizeof(u16)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_crc32(u32(reinterpret_cast<const u16&>(pod)));
    }
};
template <typename T>
struct TCRC32Hash<T, sizeof(u32)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_crc32(reinterpret_cast<const u32&>(pod));
    }
};
template <typename T>
struct TCRC32Hash<T, sizeof(u64)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_crc32(reinterpret_cast<const u64&>(pod));
    }
};
template <typename T>
struct TCRC32Hash<T, sizeof(u128)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_crc32(reinterpret_cast<const u128&>(pod));
    }
};
template <typename T>
struct TCRC32Hash<T, sizeof(u256)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_crc32(reinterpret_cast<const u256&>(pod));
    }
};
//----------------------------------------------------------------------------
// Wrap specialized hash functions for types with a small unaligned size
template <typename T>
struct TCRC32Hash<T, sizeof(u96)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return FPlatformHash::CRC32(reinterpret_cast<const u96&>(pod));
    }
};
template <typename T>
struct TCRC32Hash<T, sizeof(u160)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return FPlatformHash::CRC32(reinterpret_cast<const u160&>(pod));
    }
};
template <typename T>
struct TCRC32Hash<T, sizeof(u192)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return FPlatformHash::CRC32(reinterpret_cast<const u192&>(pod));
    }
};
template <typename T>
struct TCRC32Hash<T, sizeof(u224)> {
    static FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return FPlatformHash::CRC32(reinterpret_cast<const u224&>(pod));
    }
};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename T>
size_t hash_as_crc32(const T& pod) NOEXCEPT {
    return details::TCRC32Hash<T>::fn(pod);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
// Any pod size fall back to generic hash_mem()
template <typename T, size_t _Sz = sizeof(T)>
struct TPODHash {
    static FORCE_INLINE size_t fn(const T& pod) {
        return hash_mem(static_cast<const void*>(std::addressof(pod)), sizeof(T));
    }
};
//----------------------------------------------------------------------------
// Types with a small aligned size are using specialized hash functions
template <typename T>
struct TPODHash<T, sizeof(u8)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_uint(u32(reinterpret_cast<const u8&>(pod)));
    }
};
template <typename T>
struct TPODHash<T, sizeof(u16)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_uint(u32(reinterpret_cast<const u16&>(pod)));
    }
};
template <typename T>
struct TPODHash<T, sizeof(u32)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_uint(reinterpret_cast<const u32&>(pod));
    }
};
template <typename T>
struct TPODHash<T, sizeof(u64)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_uint(reinterpret_cast<const u64&>(pod));
    }
};
template <typename T>
struct TPODHash<T, sizeof(u128)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_uint(reinterpret_cast<const u128&>(pod));
    }
};
template <typename T>
struct TPODHash<T, sizeof(u256)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        return hash_uint(reinterpret_cast<const u256&>(pod));
    }
};
//----------------------------------------------------------------------------
// Wrap specialized hash functions for types with a small unaligned size
template <typename T>
struct TPODHash<T, sizeof(u96)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        const auto& u = reinterpret_cast<const u96&>(pod);
        return hash_uint(u128{ u.lo, u.hi });
    }
};
template <typename T>
struct TPODHash<T, sizeof(u160)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        const auto& u = reinterpret_cast<const u160&>(pod);
        return hash_uint(u256{ u.lo, { u.hi, 0 } });
    }
};
template <typename T>
struct TPODHash<T, sizeof(u192)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        const auto& u = reinterpret_cast<const u192&>(pod);
        return hash_uint(u256{ u.lo, { u.hi, 0 } });
    }
};
template <typename T>
struct TPODHash<T, sizeof(u224)> {
    static CONSTEXPR FORCE_INLINE size_t fn(const T& pod) NOEXCEPT {
        const auto& u = reinterpret_cast<const u224&>(pod);
        return hash_uint(u256{ u.lo, { u.hi.lo, u.hi.hi } });
    }
};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename T>
size_t hash_as_pod(const T& pod) NOEXCEPT {
    return details::TPODHash<T>::fn(pod);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE size_t hash_as_pod_array(const T *ptr, size_t count) NOEXCEPT {
    return hash_mem(ptr, sizeof(T) * count);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE size_t hash_as_pod_array(const T (&staticArray)[_Dim]) NOEXCEPT {
    return hash_mem(&staticArray[0], sizeof(T) * _Dim);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _It, typename _Tag>
size_t hash_as_pod_range_impl_(_It&& first, _It&& last, _Tag ) NOEXCEPT {
    size_t h = PPE_HASH_VALUE_SEED;
    for (; first != last; ++first)
        h = hash_mem(*first, sizeof(*first), h);
    return h;
}
template <typename _It>
size_t hash_as_pod_range_impl_(_It&& first, _It&& last, std::random_access_iterator_tag ) NOEXCEPT {
    return hash_as_pod_array(&(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename _It>
FORCE_INLINE size_t hash_as_pod_range(_It&& first, _It&& last) NOEXCEPT {
    typedef typename std::iterator_traits<_It>::iterator_category category;
    return details::hash_as_pod_range_impl_(std::forward<_It>(first), std::forward<_It>(last), category());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE u32 hash_mem32(const void *ptr, size_t sizeInBytes) {
    return FPlatformHash::HashMem32(PPE_HASH_VALUE_SEED_32, ptr, sizeInBytes);
}
//----------------------------------------------------------------------------
FORCE_INLINE u32 hash_mem32(const void *ptr, size_t sizeInBytes, u32 seed) {
    return FPlatformHash::HashMem32(seed, ptr, sizeInBytes);
}
//----------------------------------------------------------------------------
FORCE_INLINE u64 hash_mem64(const void *ptr, size_t sizeInBytes) {
    return FPlatformHash::HashMem64(PPE_HASH_VALUE_SEED_64, ptr, sizeInBytes);
}
//----------------------------------------------------------------------------
FORCE_INLINE u64 hash_mem64(const void *ptr, size_t sizeInBytes, u64 seed) {
    return FPlatformHash::HashMem64(seed, ptr, sizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}
