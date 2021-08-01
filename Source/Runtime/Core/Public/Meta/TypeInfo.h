#pragma once


#include "IO/ConstChar.h"
#include "IO/StaticString.h"
#include "IO/StringView.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Gets a unique ID for T,
//  /!\ *NOT* stable across different compilers/configs !
//  /!\ *NOT* guaranteed to not collide !
//----------------------------------------------------------------------------
namespace details {
template <typename T>
struct type_info_ {
    static constexpr FStringView type_name() {
        const char (&str)[sizeof(PPE_PRETTY_FUNCTION)] = PPE_PRETTY_FUNCTION;
        constexpr size_t dim = sizeof(str);
        size_t off = 1;
        for (; off <= dim && str[off - 1] != '<'; ++off);
        size_t len = 0;
        for (int balance = 1; off + len < sizeof(str) && balance; ++len) {
            if (str[off + len] == '<') ++balance;
            else if (str[off + len] == '>') --balance;
        }
        return { str + off, len };
    }
    static constexpr size_t type_hash() {
        const FStringView name = type_name();
        return hash_mem_constexpr(name.data(), name.size());
    }
    static constexpr TStaticString<type_name().size()> static_name{
        type_name()// construct a static string to trim the static string in the final executable
    };
};
} //!details
//----------------------------------------------------------------------------
struct type_info_t {
    hash_t hash_code;
    FConstChar name;
    constexpr bool operator ==(const type_info_t& other) const {
        return (hash_code == other.hash_code && name == other.name);
    }
    constexpr bool operator !=(const type_info_t& other) const {
        return ! operator ==(other);
    }
    constexpr bool operator < (const type_info_t& other) const {
        return (hash_code < other.hash_code);
    }
    constexpr bool operator >=(const type_info_t& other) const {
        return (hash_code >= other.hash_code);
    }
    constexpr friend hash_t hash_value(const type_info_t& ti) {
        return ti.hash_code;
    }
    friend void swap(type_info_t& lhs, type_info_t& rhs) noexcept {
        std::swap(lhs.hash_code, rhs.hash_code);
        std::swap(lhs.name, rhs.name);
    }
};
//----------------------------------------------------------------------------
template <typename T>
constexpr type_info_t type_info{
    details::type_info_<T>::type_hash(),
    details::type_info_<T>::static_name
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
