#include "stdafx.h"

#include "Guid.h"

#include "IO/String.h"

#ifdef OS_WINDOWS
#   include <Objbase.h>
#else
#   error "no support !"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Guid Guid::Generate() {
    Guid result;
#ifdef OS_WINDOWS
    HRESULT ret = ::CoCreateGuid(reinterpret_cast<::GUID *>(&result));
    Assert(S_OK == ret);
    UNUSED(ret);
#else
    AssertNotImplemented();
#endif
    return result;
}
//----------------------------------------------------------------------------
bool Guid::TryParse(const StringSlice& str, Guid *guid) {
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

    if (not Atoi32(&g0, StringSlice(str.SubRange(1, 8)), 16))
        return false;
    if (not Atoi32(&g1, StringSlice(str.SubRange(10, 4)), 16))
        return false;
    if (not Atoi32(&g2, StringSlice(str.SubRange(15, 4)), 16))
        return false;
    if (not Atoi32(&g3, StringSlice(str.SubRange(20, 4)), 16))
        return false;
    if (not Atoi64(&g4, StringSlice(str.SubRange(25, 12)), 16))
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
} //!namespace Core
