#pragma once

#include "Core/Core.h"

#include "Core/Time/Timestamp.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FileStat {
public:
    FileStat() : UID(0), GID(0), Link(0), Mode(0), SizeInBytes(0) {}

    u16 UID;
    u16 GID;

    u16 Link;
    u16 Mode;

    u64 SizeInBytes;

    Timestamp CreatedAt;
    Timestamp LastAccess;
    Timestamp LastModified;

    friend hash_t hash_value(const FileStat& s);
    friend void swap(FileStat& lhs, FileStat& rhs);

    static bool FromNativePath(FileStat* pstat, const wchar_t* nativeFilename);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
