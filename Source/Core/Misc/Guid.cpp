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
    ::CoCreateGuid(reinterpret_cast<::GUID *>(&result));
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

    u32 g0 = 0;
    u8  g1, g2, g3 = 0;
    u64 g4 = 0;

    if (!Atoi<16>(&g0, &str[1], 8))
        return false;
    if (!Atoi<16>(&g1, &str[10], 4))
        return false;
    if (!Atoi<16>(&g2, &str[15], 4))
        return false;
    if (!Atoi<16>(&g3, &str[20], 4))
        return false;
    if (!Atoi<16>(&g4, &str[25], 12))
        return false;

    guid->Data.as_rfc.G0 = g0;
    guid->Data.as_rfc.G1 = g1;
    guid->Data.as_rfc.G2 = g2;
    guid->Data.as_rfc.G3 = g3;
    guid->Data.as_rfc.G4 = g4;

    Assert(g0 == guid->Data.as_rfc.G0);
    Assert(g1 == guid->Data.as_rfc.G1);
    Assert(g2 == guid->Data.as_rfc.G2);
    Assert(g3 == guid->Data.as_rfc.G3);
    Assert(g4 == guid->Data.as_rfc.G4);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
