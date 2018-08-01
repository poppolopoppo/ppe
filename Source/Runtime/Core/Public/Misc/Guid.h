#pragma once

#include "Core.h"

#include "Container/Hash.h"
#include "IO/Format.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

// Globally Unique Identifier
// ex: {3F2504E0-4F89-11D3-9A0C-0305E82C3301}
// http://fr.wikipedia.org/wiki/Globally_Unique_Identifier

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ALIGN(16) struct FGuid {
    ALIGN(16) union {
        u64 as_u64[ 2];
        u32 as_u32[ 4];
        u16 as_u16[ 8];
        u8  as_u8 [16];

        struct {
            u32 G0;
            u16 G1;
            u16 G2;
            u64 G3 : 16;
            u64 G4 : 48;
        }   as_rfc;
    }   Data;

    bool empty() const;

    u64 ToUID() const { return Data.as_u64[0] ^ Data.as_u64[1]; }

    static FGuid Zero() { FGuid v; v.Data.as_u64[0] = v.Data.as_u64[1] = 0; return v; }

    PPE_API static FGuid Generate();
    PPE_API static bool TryParse(const FStringView& str, FGuid *guid);

    inline static bool TryParse(const char *str, FGuid *guid) { return TryParse(FStringView(str, Length(str)), guid); }
    template <size_t _Dim>
    static bool TryParse(const char(&cstr)[_Dim], FGuid *guid) { return TryParse(MakeStringView(cstr), guid); }

    template <size_t _Dim>
    static FGuid FromCStr(const char(&cstr)[_Dim]) {
        FGuid r = Zero();
        for (size_t i = 0; i < lengthof(r.Data.as_u8); ++i)
            r.Data.as_u8[i] = cstr[i];
        return r;
    }
};
STATIC_ASSERT(sizeof(FGuid) == 16);
//----------------------------------------------------------------------------
inline bool FGuid::empty() const {
    return  0 == Data.as_u64[0] && 0 == Data.as_u64[1];
}
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
PPE_API FTextWriter& operator <<(FTextWriter& oss, const FGuid& guid);
PPE_API FWTextWriter& operator <<(FWTextWriter& oss, const FGuid& guid);
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FGuid)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
