#pragma once

#include "Core/Meta/Aliases.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct hash_t { // see THash.h, this is fwd declared here to avoid including THash.h everywhere
public:
    size_t _value;

    hash_t()  = default;

    hash_t(size_t value) : _value(value) {}
    operator size_t () const { return _value; }

    // hash_value(hash_t) = hash_t,
    // will prevent helpers like hash_combine() from hashing twice the value :
    inline friend hash_t hash_value(hash_t value) { return value; }

    friend bool operator ==(const hash_t& lhs, const hash_t& rhs) { return lhs._value == rhs._value; }
    friend bool operator !=(const hash_t& lhs, const hash_t& rhs) { return lhs._value != rhs._value; }

    friend void swap(hash_t& lhs, hash_t& rhs) { std::swap(lhs._value, rhs._value); }

    template <typename _Traits>
    friend std::basic_ostream<char, _Traits>& operator <<(std::basic_ostream<char, _Traits>& oss, const hash_t& h) {
        return oss
            << "#0x"
            << std::setfill('0') << std::hex << std::setw(sizeof(size_t)<<1) << h._value
            << std::setfill(' ') << std::dec << std::setw(0);
    }

    template <typename _Traits>
    friend std::basic_ostream<wchar_t, _Traits>& operator <<(std::basic_ostream<wchar_t, _Traits>& oss, const hash_t& h) {
        return oss
            << L"#0x"
            << std::setfill(L'0') << std::hex << std::setw(sizeof(size_t)<<1) << h._value
            << std::setfill(L' ') << std::dec << std::setw(0);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
    // http://burtleburtle.net/bob/hash/integer.html
    constexpr u32 hash_u32_constexpr_0_(u32 a) { return u32(a^0xdeadbeefUL + (a<<4ul)); }
    constexpr u32 hash_u32_constexpr_1_(u32 a) { return u32(a ^ (a>>10ul)); }
    constexpr u32 hash_u32_constexpr_2_(u32 a) { return u32(a + (a>>24ul)); }
    constexpr u32 hash_u32_constexpr_3_(u32 a) { return u32(a + (a<< 7ul)); }
    constexpr u32 hash_u32_constexpr_4_(u32 a) { return u32(a ^ (a>>13ul)); }
}
constexpr u32 hash_u32_constexpr(u32 a) {
    return  details::hash_u32_constexpr_4_(
                details::hash_u32_constexpr_3_(
                    details::hash_u32_constexpr_2_(
                        details::hash_u32_constexpr_1_(
                            details::hash_u32_constexpr_0_( u32(a * 0x9e370001UL) )))));
}
//----------------------------------------------------------------------------
constexpr u32 hash_u32_constexpr(u32 lhs, u32 rhs) {
    // http://www.boost.org/doc/libs/1_59_0/doc/html/hash/reference.html#boost.hash_combine
    return u32(hash_u32_constexpr(lhs) ^ (hash_u32_constexpr(rhs) + 0X9E3779B9UL // 2^32 / ((1 + sqrt(5)) / 2)
        + (hash_u32_constexpr(lhs) << 6ul) + (hash_u32_constexpr(rhs) >> 2ul)) );
}
//----------------------------------------------------------------------------
template <typename... _Args>
constexpr u32 hash_u32_constexpr(u32 h0, u32 h1, u32 h2, _Args... args) {
    return hash_u32_constexpr(hash_u32_constexpr(h0, h1), h2, u32(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
    // http://burtleburtle.net/bob/hash/integer.html
    constexpr u64 hash_u64_constexpr_0_(u64 a) { return u64(a + (a>>32ull)^0x48655121ULL); }
    constexpr u64 hash_u64_constexpr_1_(u64 a) { return u64((a^0xdeadbeefabadcafeULL) + (a<<4ull)); }
    constexpr u64 hash_u64_constexpr_2_(u64 a) { return u64(a ^ (a>>10ull)); }
    constexpr u64 hash_u64_constexpr_3_(u64 a) { return u64(a + (a>>24ull)); }
    constexpr u64 hash_u64_constexpr_4_(u64 a) { return u64(a + (a<<7ull)); }
    constexpr u64 hash_u64_constexpr_5_(u64 a) { return u64(a ^ (a>>13ull)); }
}
constexpr u64 hash_u64_constexpr(u64 a) {
    return  details::hash_u64_constexpr_5_(
                details::hash_u64_constexpr_4_(
                    details::hash_u64_constexpr_3_(
                        details::hash_u64_constexpr_2_(
                            details::hash_u64_constexpr_1_(
                                details::hash_u64_constexpr_0_( u64(a * 0x9e37fffffffc0001ULL) ))))));
}
//----------------------------------------------------------------------------
constexpr u64 hash_u64_constexpr(u64 lhs, u64 rhs) {
    // http://www.boost.org/doc/libs/1_59_0/doc/html/hash/reference.html#boost.hash_combine
    return u64(hash_u64_constexpr(lhs) ^ (hash_u64_constexpr(rhs) + 0X278DDE6E5FD29E00ull // 2^64 / ((1 + sqrt(5)) / 2)
        + (hash_u64_constexpr(lhs) << 6ull) + (hash_u64_constexpr(rhs) >> 2ull)) );
}
//----------------------------------------------------------------------------
template <typename... _Args>
constexpr u64 hash_u64_constexpr(u64 h0, u64 h1, u64 h2, _Args... args) {
    return hash_u64_constexpr(hash_u64_constexpr(h0, h1), h2, u64(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef ARCH_X64
template <typename... _Args>
constexpr size_t hash_size_t_constexpr(size_t arg0, _Args... args ) {
    static_assert(sizeof(size_t) == sizeof(u64), "invalid platform !");
    return hash_u64_constexpr(u64(arg0), u64(args)...);
}
#else
template <typename... _Args>
constexpr size_t hash_size_t_constexpr(size_t arg0, _Args... args ) {
    static_assert(sizeof(size_t) == sizeof(u32), "invalid platform !");
    return hash_u32_constexpr(u32(arg0), u32(args)...);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
