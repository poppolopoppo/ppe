#pragma once

#include "Meta/Hash_fwd.h"
#include "Meta/Warnings.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Gets a unique ID for T,
//  /!\ *NOT* stable across different compilers/configs !
//  /!\ *NOT* guaranteed to not collide !
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4307) // '*' : integral constant overflow
template <typename T>
struct type_id {
    static constexpr hash_t make_uid() noexcept {
        constexpr const char* const name = PPE_PRETTY_FUNCTION;

        size_t uid = 0; // static hash of this function signature
        for (const char* ch = name; *ch; ++ch)
            uid = hash_size_t_constexpr(uid, *ch);

        return hash_t{ uid };
    }
    static constexpr hash_t value = make_uid(); // fully static/constexpr
};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
