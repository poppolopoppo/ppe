#pragma once

#include "Core/Core.h"

#include "Core/Time/Timestamp.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFileStat {
public:
    FFileStat() : UID(0), GID(0), Link(0), Mode(0), SizeInBytes(0) {}

    u16 UID;
    u16 GID;

    u16 Link;
    u16 Mode;

    u64 SizeInBytes;

    FTimestamp CreatedAt;
    FTimestamp LastAccess;
    FTimestamp LastModified;

    friend hash_t hash_value(const FFileStat& s);
    friend void swap(FFileStat& lhs, FFileStat& rhs);

    static bool FromNativePath(FFileStat* pstat, const wchar_t* nativeFilename);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
