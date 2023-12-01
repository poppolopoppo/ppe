#pragma once

#include "Core.h"

#include "Container/Hash.h"
#include "IO/Format.h"
#include "IO/String_fwd.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

// Globally Unique Identifier
// ex: {3F2504E0-4F89-11D3-9A0C-0305E82C3301}
// http://fr.wikipedia.org/wiki/Globally_Unique_Identifier

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct ALIGN(16) FGuid {
    ALIGN(16) union {
        u32 as_u32[ 4];
        u64 as_u64[ 2];
        u16 as_u16[ 8];
        u8  as_u8 [16];

        u128 as_u128;

        struct {
            u32 G0;
            u16 G1;
            u16 G2;
            u64 G3 : 16;
            u64 G4 : 48;
        }   as_rfc;
    }   Data;

    CONSTEXPR u64 ToUID() const { return Data.as_u64[0] ^ Data.as_u64[1]; }

    FString ToString() const;
    FWString ToWString() const;

    CONSTEXPR bool empty() const { return (Data.as_u64[0] == 0 && Data.as_u64[1] == 0); }

    CONSTEXPR static FGuid Zero() { return { {{0, 0, 0, 0 }} }; }

    CONSTEXPR static FGuid Binary(u32 a, u32 b, u32 c, u32 d) {
        return { {{ a, b, c, d }} };
    }

    PPE_CORE_API static FGuid Generate() NOEXCEPT;
    PPE_CORE_API static bool TryParse(const FStringView& str, FGuid *guid) NOEXCEPT;

    inline static bool TryParse(const char *str, FGuid *guid) { return TryParse(FStringView(str, Length(str)), guid); }
    template <size_t _Dim>
    static bool TryParse(const char(&cstr)[_Dim], FGuid *guid) { return TryParse(MakeStringView(cstr), guid); }

};
STATIC_ASSERT(sizeof(FGuid) == 16);
//----------------------------------------------------------------------------
inline bool operator ==(const FGuid& lhs, const FGuid& rhs) {
    return  lhs.Data.as_u64[0] == rhs.Data.as_u64[0] &&
            lhs.Data.as_u64[1] == rhs.Data.as_u64[1];
}
//----------------------------------------------------------------------------
inline bool operator !=(const FGuid& lhs, const FGuid& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline bool operator < (const FGuid& lhs, const FGuid& rhs) {
    if (lhs.Data.as_u64[0] < rhs.Data.as_u64[0])
        return true;
    else if (lhs.Data.as_u64[0] > rhs.Data.as_u64[0])
        return false;
    else if (lhs.Data.as_u64[1] < rhs.Data.as_u64[1])
        return true;
    else
        return false;
}
//----------------------------------------------------------------------------
inline bool operator >=(const FGuid& lhs, const FGuid& rhs) {
    return !operator < (lhs, rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FGuid& guid) {
    return hash_as_pod(guid.Data.as_u64);
}
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FGuid& guid);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FGuid& guid);
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FGuid)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
