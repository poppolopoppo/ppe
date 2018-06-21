#include "stdafx.h"

#include "Guid.h"

#include "IO/String.h"
#include "IO/TextWriter.h"

#ifdef PLATFORM_WINDOWS
#   include <Objbase.h>
#else
#   error "no support !"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGuid FGuid::Generate() {
    FGuid result;
#ifdef PLATFORM_WINDOWS
    HRESULT ret = ::CoCreateGuid(reinterpret_cast<::GUID *>(&result));
    Assert(S_OK == ret);
    UNUSED(ret);
#else
    AssertNotImplemented();
#endif
    return result;
}
//----------------------------------------------------------------------------
bool FGuid::TryParse(const FStringView& str, FGuid *guid) {
    Assert(guid);

    // Format: {88888888-4444-4444-4444-444488888888}

    if (str.empty() ||
        str.size() != 38 ||
        str.front() != '{' ||
        str[9] != '-' || str[14] != '-' || str[19] != '-' || str[24] != '-' ||
        str.back() != '}' )
        return false;

    i32 g0 = 0;
    i32 g1, g2, g3 = 0;
    i64 g4 = 0;

    if (not Atoi(&g0, FStringView(str.SubRange(1, 8)), 16))
        return false;
    if (not Atoi(&g1, FStringView(str.SubRange(10, 4)), 16))
        return false;
    if (not Atoi(&g2, FStringView(str.SubRange(15, 4)), 16))
        return false;
    if (not Atoi(&g3, FStringView(str.SubRange(20, 4)), 16))
        return false;
    if (not Atoi(&g4, FStringView(str.SubRange(25, 12)), 16))
        return false;

    guid->Data.as_rfc.G0 = g0;
    guid->Data.as_rfc.G1 = checked_cast<u8>(g1);
    guid->Data.as_rfc.G2 = checked_cast<u8>(g2);
    guid->Data.as_rfc.G3 = checked_cast<u8>(g3);
    guid->Data.as_rfc.G4 = g4;

    Assert(u32(g0) == guid->Data.as_rfc.G0);
    Assert(u16(g1) == guid->Data.as_rfc.G1);
    Assert(u16(g2) == guid->Data.as_rfc.G2);
    Assert(u64(g3) == guid->Data.as_rfc.G3);
    Assert(u64(g4) == guid->Data.as_rfc.G4);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FGuid& guid) {
#if 0
    Format(oss, "{{0:#8X}-{1:#4X}-{2:#4X}-{3:#4X}-{4:#12X}}",
        guid.Data.as_rfc.G0,
        guid.Data.as_rfc.G1,
        guid.Data.as_rfc.G2,
        guid.Data.as_rfc.G3,
        guid.Data.as_rfc.G4);
#else
    const FTextFormat fmt = oss.Format();
    oss << '{'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(8, '0') << guid.Data.as_rfc.G0
        << '-'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, '0') << guid.Data.as_rfc.G1
        << '-'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, '0') << guid.Data.as_rfc.G2
        << '-'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, '0') << guid.Data.as_rfc.G3
        << '-'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(12, '0') << guid.Data.as_rfc.G4
        << '}'
        << fmt;
#endif
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FGuid& guid) {
    const FTextFormat fmt = oss.Format();
    oss << L'{'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(8, L'0') << guid.Data.as_rfc.G0
        << L'-'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, L'0') << guid.Data.as_rfc.G1
        << L'-'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, L'0') << guid.Data.as_rfc.G2
        << L'-'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(4, L'0') << guid.Data.as_rfc.G3
        << L'-'
        << FTextFormat::Hexadecimal << FTextFormat::PadLeft(12, L'0') << guid.Data.as_rfc.G4
        << '}'
        << fmt;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
