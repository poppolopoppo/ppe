#pragma once

#include "Core_fwd.h"

#include "Meta/Hash_fwd.h"
#include "HAL/PlatformHash.h"
#include "Memory/HashFunctions.h"
#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"
#include "Meta/TypeTraits.h"

#include <type_traits>

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4307) // '*': integral constant overflow

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR CONSTF hash_t hash_value(bool v)     NOEXCEPT { return hash_uint(static_cast<size_t>(v)); }
CONSTEXPR CONSTF hash_t hash_value(i8 v)       NOEXCEPT { return hash_uint(static_cast<size_t>(v)); }
CONSTEXPR CONSTF hash_t hash_value(u8 v)       NOEXCEPT { return hash_uint(static_cast<size_t>(v)); }
CONSTEXPR CONSTF hash_t hash_value(i16 v)      NOEXCEPT { return hash_uint(static_cast<size_t>(v)); }
CONSTEXPR CONSTF hash_t hash_value(u16 v)      NOEXCEPT { return hash_uint(static_cast<size_t>(v)); }
CONSTEXPR CONSTF hash_t hash_value(i32 v)      NOEXCEPT { return hash_uint(static_cast<size_t>(v)); }
CONSTEXPR CONSTF hash_t hash_value(u32 v)      NOEXCEPT { return hash_uint(static_cast<size_t>(v)); }
CONSTEXPR CONSTF hash_t hash_value(i64 v)      NOEXCEPT { return hash_uint(static_cast<u64>(v)); }
CONSTEXPR CONSTF hash_t hash_value(u64 v)      NOEXCEPT { return hash_uint(v); }
CONSTEXPR CONSTF hash_t hash_value(u128 v)     NOEXCEPT { return hash_uint(v); }
FORCE_INLINE CONSTF hash_t hash_value(u256 v)  NOEXCEPT { return hash_as_pod(v); }
FORCE_INLINE CONSTF hash_t hash_value(float v) NOEXCEPT { return hash_uint(bit_cast<u32>(v)); }
FORCE_INLINE CONSTF hash_t hash_value(double v)NOEXCEPT { return hash_uint(bit_cast<u64>(v)); }
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE CONSTF hash_t hash_value(const T *ptr) NOEXCEPT {
    return hash_ptr(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Helpers for combining hash values:
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR CONSTF void hash_combine(hash_t& seed, const T& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR CONSTF void hash_range(hash_t& seed, const T *values, size_t count) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR CONSTF hash_t hash_range(const T *values, size_t count) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR CONSTF hash_t hash_range(_It first, _It last) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR CONSTF hash_t hash_range(const TIterable<_It>& range) NOEXCEPT {
    return hash_range(range.begin(), range.end());
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
CONSTEXPR CONSTF void hash_combine(hash_t& seed, const _Arg0& arg0, const _Args&... args) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
CONSTEXPR CONSTF hash_t hash_tuple(_Arg0&& arg0, _Args&&... args) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR CONSTF hash_t hash_view(const TMemoryView<T>& view) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR CONSTF hash_t& operator <<(hash_t& seed, const T& value) NOEXCEPT {
    hash_combine(seed, value);
    return seed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Meta {
template <typename T = void>
struct THash {
    CONSTEXPR CONSTF hash_t operator ()(const T& value) const NOEXCEPT {
        using PPE::hash_value;
        return hash_value(value);
    }
};
template <>
struct THash<void> {
    template <typename T>
    CONSTEXPR CONSTF hash_t operator ()(const T& value) const NOEXCEPT {
        using PPE::hash_value;
        return hash_value(value);
    }
};
template <typename T = void>
struct TCRC32 {
    CONSTEXPR CONSTF hash_t operator ()(const T& value) const NOEXCEPT {
        using PPE::hash_as_crc32;
        return hash_as_crc32(value);
    }
};
template <>
struct TCRC32<void> {
    template <typename T>
    CONSTEXPR CONSTF hash_t operator ()(const T& value) const NOEXCEPT {
        using PPE::hash_as_crc32;
        return hash_as_crc32(value);
    }
};
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/Hash-inl.h"

PRAGMA_MSVC_WARNING_POP()
