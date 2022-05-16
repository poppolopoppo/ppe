#pragma once

#include "Meta/Aliases.h"
#include "Meta/ForRange.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define _PPE_DETAILS_AUTOSTRUCT_DEFINE(_FIELD_TYPE, _FIELD_NAME) _FIELD_TYPE _FIELD_NAME;
#define _PPE_DETAILS_AUTOSTRUCT_EQUALS(_FIELD_TYPE, _FIELD_NAME) and (lhs._FIELD_NAME == rhs._FIELD_NAME)
#define _PPE_DETAILS_AUTOSTRUCT_HASHVALUE(_FIELD_TYPE, _FIELD_NAME) , value._FIELD_NAME
//----------------------------------------------------------------------------
#define PPE_AUTOSTRUCT_MEMBERS(_STRUCT_NAME, ...) \
    PP_FOREACH_TUPLE(_PPE_DETAILS_AUTOSTRUCT_DEFINE, __VA_ARGS__) \
    friend bool operator !=(const _STRUCT_NAME& lhs, const _STRUCT_NAME& rhs) NOEXCEPT { return not operator ==(lhs, rhs); } \
    friend bool operator ==(const _STRUCT_NAME& lhs, const _STRUCT_NAME& rhs) NOEXCEPT { \
        return (true PP_FOREACH_TUPLE(_PPE_DETAILS_AUTOSTRUCT_EQUALS, __VA_ARGS__)); \
    } \
    friend PPE::hash_t hash_value(const _STRUCT_NAME& value) NOEXCEPT { \
        return PPE::hash_tuple( PPE::hash_str_constexpr(STRINGIZE(_STRUCT_NAME)) \
            PP_FOREACH_TUPLE(_PPE_DETAILS_AUTOSTRUCT_HASHVALUE, __VA_ARGS__)); \
    }

//----------------------------------------------------------------------------
#define PPE_DEFINE_AUTOSTRUCT(_STRUCT_NAME, ...) struct _STRUCT_NAME { \
        PPE_AUTOSTRUCT_MEMBERS(_STRUCT_NAME, __VA_ARGS__) \
    };
//----------------------------------------------------------------------------
#define PPE_DEFINE_AUTOPOD(_STRUCT_NAME, ...) struct _STRUCT_NAME { \
        PPE_AUTOSTRUCT_MEMBERS(_STRUCT_NAME, __VA_ARGS__) \
        PPE_ASSUME_FRIEND_AS_POD(_STRUCT_NAME) \
    };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
