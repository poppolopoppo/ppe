#pragma once

#include "Core/Core.h"

#include "Core/Container/Hash.h"
#include "Core/IO/Format.h"
#include "Core/IO/StringSlice.h"

// Globally Unique Identifier
// ex: {3F2504E0-4F89-11D3-9A0C-0305E82C3301}
// http://fr.wikipedia.org/wiki/Globally_Unique_Identifier

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ALIGN(16) struct Guid {
    ALIGN(16) union {
        u64 as_u64[2];
        u32 as_u32[4];
        u16 as_u16[8];
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

    static Guid Zero() { return Guid{0, 0}; }

    static Guid Generate();
    static bool TryParse(const StringSlice& str, Guid *guid);

    static bool TryParse(const char *str, Guid *guid) { return TryParse(StringSlice(str, Length(str)), guid); }
    template <size_t _Dim>
    static bool TryParse(const char(&cstr)[_Dim], Guid *guid) { return TryParse(MakeStringSlice(cstr), guid); }

    template <size_t _Dim>
    static Guid FromCStr(const char(&cstr)[_Dim]) {
        Guid r = Zero();
        for (size_t i = 0; i < lengthof(r.Data.as_u8); ++i)
            r.Data.as_u8[i] = cstr[i];
        return r;
    }
};
STATIC_ASSERT(sizeof(Guid) == 16);
//----------------------------------------------------------------------------
inline bool Guid::empty() const {
    return  0 == Data.as_u64[0] && 0 == Data.as_u64[1];
}
//----------------------------------------------------------------------------
inline bool operator ==(const Guid& lhs, const Guid& rhs) {
    return  lhs.Data.as_u64[0] == rhs.Data.as_u64[0] &&
            lhs.Data.as_u64[1] == rhs.Data.as_u64[1];
}
//----------------------------------------------------------------------------
inline bool operator !=(const Guid& lhs, const Guid& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline bool operator < (const Guid& lhs, const Guid& rhs) {
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
inline bool operator >=(const Guid& lhs, const Guid& rhs) {
    return !operator < (lhs, rhs);
}
//----------------------------------------------------------------------------
inline size_t hash_value(const Guid& guid) {
    return hash_value_seq(&guid.Data.as_u64[0], &guid.Data.as_u64[1]);
}
//----------------------------------------------------------------------------
template <typename _Traits>
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const Guid& guid ) {
    Format( oss, "{{0:#8X}-{1:#4X}-{2:#4X}-{3:#4X}-{4:#12X}}",
            guid.Data.as_rfc.G0,
            guid.Data.as_rfc.G1,
            guid.Data.as_rfc.G2,
            guid.Data.as_rfc.G3,
            guid.Data.as_rfc.G4 );
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Traits>
std::basic_ostream<wchar_t, _Traits>& operator <<(
    std::basic_ostream<wchar_t, _Traits>& oss,
    const Guid& guid ) {
    Format( oss, L"{{0:#8X}-{1:#4X}-{2:#4X}-{3:#4X}-{4:#12X}}",
            guid.Data.as_rfc.G0,
            guid.Data.as_rfc.G1,
            guid.Data.as_rfc.G2,
            guid.Data.as_rfc.G3,
            guid.Data.as_rfc.G4 );
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
