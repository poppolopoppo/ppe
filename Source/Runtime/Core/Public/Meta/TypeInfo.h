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
struct type_info_parser_t {
    static constexpr FStringView type_name() {
        const char (&fun)[lengthof(PPE_PRETTY_FUNCTION)] = PPE_PRETTY_FUNCTION;
        const char (&suf)[lengthof("type_info_parser_t<")] = "type_info_parser_t<";
        // find suffix function name
        size_t first = 0;
        for (; first + lengthof(suf) - 1 <= lengthof(fun) - 1; ++first) {
            size_t subl = 0;
            for (; subl < lengthof(suf) - 1 && fun[first + subl] == suf[subl]; ++subl);
            if (subl == lengthof(suf) - 1) {
                first += subl;
                break;
            }
        }
        // select balanced
        size_t last = first;
        for (int balance = 1; last < lengthof(fun) - 1 && balance; ++last) {
            if (fun[last] == '<') ++balance;
            else if (fun[last] == '>') --balance;
        }
        // trim spaces
        for (; last > first && fun[last - 1] == ' '; --last);
        for (; first < last && fun[first] == ' '; ++first);
        // return only the interesting part
        return { fun + first, last - first - 1 };
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
    details::type_info_parser_t<T>::type_uid(),
    details::type_info_parser_t<T>::static_name
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
