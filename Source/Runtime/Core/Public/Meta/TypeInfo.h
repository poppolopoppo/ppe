#pragma once

#include "Meta/Hash_fwd.h"
#include "Meta/MD5.h"

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
    static constexpr FMd5sum type_uid() {
        return md5sum(type_name());
    }
    static constexpr TStaticString<type_name().size()> static_name{
        type_name()// construct a static string to trim the static string in the final executable
    };
};
} //!details
//----------------------------------------------------------------------------
struct type_info_t {
    FMd5sum uid;
    FConstChar name;
    constexpr bool operator ==(const type_info_t& other) const {
        Assert_NoAssume(uid != other.uid || name == other.name); // check for collisions
        return (uid == other.uid);
    }
    constexpr bool operator !=(const type_info_t& other) const {
        return (not operator ==(other));
    }
    constexpr bool operator < (const type_info_t& other) const {
        return (uid <  other.uid);
    }
    constexpr bool operator >=(const type_info_t& other) const {
        return (not operator < (other));
    }
    constexpr friend hash_t hash_value(const type_info_t& ti) {
        return hash_u32_constexpr(ti.uid[0], ti.uid[1], ti.uid[2], ti.uid[3]);
    }
    friend void swap(type_info_t& lhs, type_info_t& rhs) noexcept {
        std::swap(lhs.uid, rhs.uid);
        std::swap(lhs.name, rhs.name);
    }
};
//----------------------------------------------------------------------------
template <typename T>
constexpr type_info_t type_info{
    details::type_info_<T>::type_uid(),
    details::type_info_<T>::static_name
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
