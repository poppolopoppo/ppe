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
    size_t _value;

    hash_t() = default;

    CONSTEXPR hash_t(size_t value) : _value(value) {}
    CONSTEXPR operator size_t () const { return _value; }

    // hash_value(hash_t) = hash_t,
    // will prevent helpers like hash_combine() from hashing twice the value :
    CONSTEXPR inline friend hash_t hash_value(hash_t value) { return value; }

    CONSTEXPR friend bool operator ==(const hash_t& lhs, const hash_t& rhs) { return lhs._value == rhs._value; }
    CONSTEXPR friend bool operator !=(const hash_t& lhs, const hash_t& rhs) { return lhs._value != rhs._value; }

    friend void swap(hash_t& lhs, hash_t& rhs) { std::swap(lhs._value, rhs._value); }
};
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, hash_t h);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, hash_t h);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if _HAS_CXX14
//----------------------------------------------------------------------------
CONSTEXPR u32 hash_u32_constexpr(u32 h32) NOEXCEPT {
    h32 ^= h32 >> 15; // XXH32_avalanche()
    h32 *= 2246822519U;
    h32 ^= h32 >> 13;
    h32 *= 3266489917U;
    h32 ^= h32 >> 16;
    return h32;
}
//----------------------------------------------------------------------------
CONSTEXPR u32 hash_u32_constexpr(u32 h, u32 k) NOEXCEPT {
    k *= 0xcc9e2d51ul; // https://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
    k = (k << 15ul) | (k >> (32ul - 15ul));
    k *= 0x1b873593ul;
    h ^= k;
    h = (h << 13ul) | (h >> (32ul - 13ul));
    h = h * 5ul + 0xe6546b64ul;
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
#endif //!_HAS_CXX14
//----------------------------------------------------------------------------
// recursion :
template <typename... _Args>
CONSTEXPR u32 hash_u32_constexpr(u32 h0, u32 h1, u32 h2, _Args... args) NOEXCEPT {
    return hash_u32_constexpr(hash_u32_constexpr(h0, h1), h2, u32(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if _HAS_CXX14
//----------------------------------------------------------------------------
CONSTEXPR u64 hash_u64_constexpr(u64 h64) NOEXCEPT {
    h64 ^= h64 >> 33; // XXH64_avalanche()
    h64 *= 14029467366897019727ULL;
    h64 ^= h64 >> 29;
    h64 *= 1609587929392839161ULL;
    h64 ^= h64 >> 32;
    return h64;
}
//----------------------------------------------------------------------------
CONSTEXPR u64 hash_u64_constexpr(u64 h, u64 k) NOEXCEPT {
    k *= 0xc6a4a7935bd1e995ull; // https://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
    k ^= k >> 47ull;
    k *= 0xc6a4a7935bd1e995ull;
    h ^= k;
    h *= 0xc6a4a7935bd1e995ull;
    h += 0xe6546b64ull; // Completely arbitrary number, to prevent 0's from hashing to 0.
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
#endif //!_HAS_CXX14
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
