#include "stdafx.h"

#include "FileStat.h"

#include "Core/Memory/HashFunctions.h"

#ifdef OS_WINDOWS
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <wchar.h>
#else
#   error "unsupported platform"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const FFileStat& s) {
    return hash_as_pod(s);
}
//----------------------------------------------------------------------------
void swap(FFileStat& lhs, FFileStat& rhs) {
    using std::swap;
    swap(lhs.UID, rhs.UID);
    swap(lhs.GID, rhs.GID);
    swap(lhs.Link, rhs.Link);
    swap(lhs.Mode, rhs.Mode);
    swap(lhs.SizeInBytes, rhs.SizeInBytes);
    swap(lhs.CreatedAt, rhs.CreatedAt);
    swap(lhs.LastAccess, rhs.LastAccess);
    swap(lhs.LastModified, rhs.LastModified);
}
//----------------------------------------------------------------------------
bool FFileStat::FromNativePath(FFileStat* pstat, const wchar_t* nativeFilename) {
    Assert(pstat);
    Assert(nativeFilename);

    struct ::_stat64 fs;
    if (0 != ::_wstat64(nativeFilename, &fs)) {
        *pstat = FFileStat();
        return false;
    }

    pstat->UID = checked_cast<u16>(fs.st_uid);
    pstat->GID = checked_cast<u16>(fs.st_gid);
    pstat->Link = checked_cast<u16>(fs.st_nlink);
    pstat->Mode = checked_cast<u16>(fs.st_mode);

    pstat->SizeInBytes = checked_cast<u64>(fs.st_size);

    pstat->CreatedAt.SetValue(fs.st_ctime);
    pstat->LastAccess.SetValue(fs.st_atime);
    pstat->LastModified.SetValue(fs.st_mtime);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
