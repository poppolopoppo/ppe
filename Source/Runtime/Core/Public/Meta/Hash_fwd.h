#pragma once

#include "Core_fwd.h"

#include "IO/TextWriter_fwd.h"
#include "Meta/Aliases.h"
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct hash_t { // see Hash.h, this is fwd declared here to avoid including Hash.h everywhere
public:
    using value_type = std::size_t;

    value_type _value;

    hash_t() = default;
    CONSTEXPR hash_t(Meta::FForceInit) NOEXCEPT : _value(0) {}
    CONSTEXPR hash_t(value_type value) NOEXCEPT : _value(value) {}
    CONSTEXPR operator value_type () const { return _value; }

    // hash_value(hash_t) = hash_t,
    // will prevent helpers like hash_combine() from hashing twice the value :
    CONSTEXPR inline friend hash_t hash_value(hash_t value) NOEXCEPT { return value; }

    CONSTEXPR friend bool operator ==(const hash_t& lhs, const hash_t& rhs) { return lhs._value == rhs._value; }
    CONSTEXPR friend bool operator !=(const hash_t& lhs, const hash_t& rhs) { return lhs._value != rhs._value; }

    friend void swap(hash_t& lhs, hash_t& rhs) NOEXCEPT { std::swap(lhs._value, rhs._value); }
};
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, hash_t h);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, hash_t h);
//----------------------------------------------------------------------------
// Understanding the Linux Kernel
// https://books.google.fr/books?id=h0lltXyJ8aIC&lpg=PT109&ots=gO2uM_c7FQ&dq=The%20Magic%20Constant%20linux%20hash&hl=fr&pg=PT109#v=onepage&q=The%20Magic%20Constant%20linux%20hash&f=false
// 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL
// 2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1
#define GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001ULL
//----------------------------------------------------------------------------
#define GOLDEN_RATIO_PRIME CODE3264(GOLDEN_RATIO_PRIME_32, GOLDEN_RATIO_PRIME_64)
//----------------------------------------------------------------------------
// Random generated hash value seeds :
// '0x' << Array.new(8).collect!{ rand(16).to_s(16).upcase }.join << 'ul'
#define PPE_HASH_VALUE_SEED_32 0xD8D40566ull
// '0x' << Array.new(16).collect!{ rand(16).to_s(16).upcase }.join << 'ull'
#define PPE_HASH_VALUE_SEED_64 0x829787DB44E899F5ull
//----------------------------------------------------------------------------
#define PPE_HASH_VALUE_SEED CODE3264(PPE_HASH_VALUE_SEED_32, PPE_HASH_VALUE_SEED_64)
//----------------------------------------------------------------------------
// See HashFunctions.h for the definition of those functions:
template <typename T>
size_t hash_as_pod(const T& pod) NOEXCEPT;
size_t hash_mem(const void *ptr, size_t sizeInBytes) NOEXCEPT;
size_t hash_ptr(const void* ptr) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE auto hash_value(T value) NOEXCEPT -> typename std::enable_if_t<std::is_enum<T>::value, hash_t> {
    return hash_as_pod(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if PPE_HAS_CXX14
//----------------------------------------------------------------------------
CONSTEXPR u32 hash_u32_constexpr(u32 h32) NOEXCEPT {
    h32 ^= h32 >> u32(15); // XXH32_avalanche()
    h32 *= u32(2246822519UL);
    h32 ^= h32 >> u32(13);
    h32 *= u32(3266489917UL);
    h32 ^= h32 >> u32(16);
    return h32;
}
//----------------------------------------------------------------------------
CONSTEXPR u32 hash_u32_constexpr(u32 h, u32 k) NOEXCEPT {
    k *= u32(0xCC9E2D51UL); // https://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
    k = (k << u32(15)) | (k >> u32(32 - 15));
    k *= u32(0x1B873593UL);
    h ^= k;
    h = (h << u32(13)) | (h >> u32(32 - 13));
    h = h * u32(5) + u32(0xE6546B64UL);
    return h;
}
//----------------------------------------------------------------------------
#else // support for C++11 CONSTEXPR :
//----------------------------------------------------------------------------
namespace details {
    // http://burtleburtle.net/bob/hash/integer.html
    CONSTEXPR u32 hash_u32_constexpr_0_(u32 a) { return u32(a^0xdeadbeefUL + (a<<4ul)); }
    CONSTEXPR u32 hash_u32_constexpr_1_(u32 a) { return u32(a ^ (a>>10ul)); }
    CONSTEXPR u32 hash_u32_constexpr_2_(u32 a) { return u32(a + (a>>24ul)); }
    CONSTEXPR u32 hash_u32_constexpr_3_(u32 a) { return u32(a + (a<< 7ul)); }
    CONSTEXPR u32 hash_u32_constexpr_4_(u32 a) { return u32(a ^ (a>>13ul)); }
}
CONSTEXPR u32 hash_u32_constexpr(u32 a) {
    return  details::hash_u32_constexpr_4_(
                details::hash_u32_constexpr_3_(
                    details::hash_u32_constexpr_2_(
                        details::hash_u32_constexpr_1_(
                            details::hash_u32_constexpr_0_( u32(a * 0x9e370001UL) )))));
}
//----------------------------------------------------------------------------
CONSTEXPR u32 hash_u32_constexpr(u32 lhs, u32 rhs) {
    // http://www.boost.org/doc/libs/1_59_0/doc/html/hash/reference.html#boost.hash_combine
    return u32(hash_u32_constexpr(lhs) ^ (hash_u32_constexpr(rhs) + 0X9E3779B9UL // 2^32 / ((1 + sqrt(5)) / 2)
        + (hash_u32_constexpr(lhs) << 6ul) + (hash_u32_constexpr(rhs) >> 2ul)) );
}
//----------------------------------------------------------------------------
#endif //!PPE_HAS_CXX14
//----------------------------------------------------------------------------
// recursion :
template <typename... _Args>
CONSTEXPR u32 hash_u32_constexpr(u32 h0, u32 h1, u32 h2, _Args... args) NOEXCEPT {
    return hash_u32_constexpr(hash_u32_constexpr(h0, h1), h2, u32(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if PPE_HAS_CXX14
//----------------------------------------------------------------------------
CONSTEXPR u64 hash_u64_constexpr(u64 h64) NOEXCEPT {
    h64 ^= h64 >> u64(33); // XXH64_avalanche()
    h64 *= u64(14029467366897019727ULL);
    h64 ^= h64 >> u64(29);
    h64 *= u64(1609587929392839161ULL);
    h64 ^= h64 >> u64(32);
    return h64;
}
//----------------------------------------------------------------------------
CONSTEXPR u64 hash_u64_constexpr(u64 h, u64 k) NOEXCEPT {
    k *= u64(0xC6A4A7935BD1E995ULL); // https://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
    k ^= k >> u64(47);
    k *= u64(0xC6A4A7935BD1E995ULL);
    h ^= k;
    h *= u64(0xC6A4A7935BD1E995ULL);
    h += u64(0xE6546B64ULL); // Completely arbitrary number, to prevent 0's from hashing to 0.
    return h;
}
//----------------------------------------------------------------------------
#else // support for C++11 CONSTEXPR :
//----------------------------------------------------------------------------
namespace details {
    // http://burtleburtle.net/bob/hash/integer.html
    CONSTEXPR u64 hash_u64_constexpr_0_(u64 a) { return u64(a + (a>>32ull)^0x48655121ULL); }
    CONSTEXPR u64 hash_u64_constexpr_1_(u64 a) { return u64((a^0xdeadbeefabadcafeULL) + (a<<4ull)); }
    CONSTEXPR u64 hash_u64_constexpr_2_(u64 a) { return u64(a ^ (a>>10ull)); }
    CONSTEXPR u64 hash_u64_constexpr_3_(u64 a) { return u64(a + (a>>24ull)); }
    CONSTEXPR u64 hash_u64_constexpr_4_(u64 a) { return u64(a + (a<<7ull)); }
    CONSTEXPR u64 hash_u64_constexpr_5_(u64 a) { return u64(a ^ (a>>13ull)); }
}
CONSTEXPR u64 hash_u64_constexpr(u64 a) {
    return  details::hash_u64_constexpr_5_(
                details::hash_u64_constexpr_4_(
                    details::hash_u64_constexpr_3_(
                        details::hash_u64_constexpr_2_(
                            details::hash_u64_constexpr_1_(
                                details::hash_u64_constexpr_0_( u64(a * 0x9e37fffffffc0001ULL) ))))));
}
//----------------------------------------------------------------------------
CONSTEXPR u64 hash_u64_constexpr(u64 lhs, u64 rhs) {
    // http://www.boost.org/doc/libs/1_59_0/doc/html/hash/reference.html#boost.hash_combine
    return u64(hash_u64_constexpr(lhs) ^ (hash_u64_constexpr(rhs) + 0X278DDE6E5FD29E00ull // 2^64 / ((1 + sqrt(5)) / 2)
        + (hash_u64_constexpr(lhs) << 6ull) + (hash_u64_constexpr(rhs) >> 2ull)) );
}
//----------------------------------------------------------------------------
#endif //!PPE_HAS_CXX14
//----------------------------------------------------------------------------
template <typename... _Args>
CONSTEXPR u64 hash_u64_constexpr(u64 h0, u64 h1, u64 h2, _Args... args) NOEXCEPT {
    return hash_u64_constexpr(hash_u64_constexpr(h0, h1), h2, u64(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef ARCH_X64
template <typename... _Args>
CONSTEXPR size_t hash_size_t_constexpr(size_t arg0, _Args... args ) NOEXCEPT {
    static_assert(sizeof(size_t) == sizeof(u64), "invalid platform !");
    return hash_u64_constexpr(u64(arg0), u64(args)...);
}
#else
template <typename... _Args>
CONSTEXPR size_t hash_size_t_constexpr(size_t arg0, _Args... args ) NOEXCEPT {
    static_assert(sizeof(size_t) == sizeof(u32), "invalid platform !");
    return hash_u32_constexpr(u32(arg0), u32(args)...);
}
#endif //!ARCH_X64
//----------------------------------------------------------------------------
template <typename T, size_t... _Indices>
CONSTEXPR size_t hash_sequence_constexpr(const T* data, std::index_sequence<_Indices...>) NOEXCEPT {
    return hash_size_t_constexpr(sizeof...(_Indices)/* handles hash_sequence_constexpr(nullptr, 0) */, data[_Indices]...);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR size_t hash_mem_constexpr(const T* data, size_t n) NOEXCEPT {
    size_t h = hash_size_t_constexpr(n);
    for (size_t i = 0; i < n; ++i)
        h = hash_size_t_constexpr(h, data[i]);
    return h;
}
//----------------------------------------------------------------------------
template <typename T, size_t N>
CONSTEXPR size_t hash_arr_constexpr(const T (&arr)[N]) NOEXCEPT {
    return hash_mem_constexpr(arr, sizeof(T) * N);
}
//----------------------------------------------------------------------------
template <typename _Char>
CONSTEXPR size_t hash_str_constexpr(const _Char* str) NOEXCEPT {
    Assert(str);
    size_t h = 0, n = 0;
    for (; *str; ++str, ++n)
        h = hash_size_t_constexpr(h, *str);
    return hash_size_t_constexpr(h, n);
}
//----------------------------------------------------------------------------
CONSTEXPR size_t hash_strI_constexpr(const char* str) NOEXCEPT {
    Assert(str);
    size_t h = 0, n = 0;
    for (; *str; ++str, ++n)
        h = hash_size_t_constexpr(h, ((*str >= 'A') && (*str <= 'Z')) ? 'a' + (*str - 'A') : *str);
    return hash_size_t_constexpr(h, n);
}
//----------------------------------------------------------------------------
template <typename _FwdIt>
CONSTEXPR size_t hash_fwdit_constexpr(_FwdIt first, _FwdIt last) NOEXCEPT {
    size_t h = hash_size_t_constexpr(*(first++));
    for (; first != last; ++first)
        h = hash_size_t_constexpr(h, *first);
    return h;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
