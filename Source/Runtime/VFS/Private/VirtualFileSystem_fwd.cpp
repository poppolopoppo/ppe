#include "stdafx.h"

#include "VirtualFileSystem_fwd.h"

#include "VirtualFileSystem.h"

#include "Container/RawStorage.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/FileStream.h"
#include "IO/Format.h"
#include "IO/TextWriter.h"
#include "HAL/PlatformFile.h"
#include "Misc/Function.h"
#include "Time/DateTime.h"
#include "Time/Timestamp.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemTrie& VFS() {
    return FVirtualFileSystem::Get();
}
//----------------------------------------------------------------------------
bool VFS_DirectoryExists(const FDirpath& dirpath, EExistPolicy policy/* = ExistPolicy::Exists */) {
    return VFS().DirectoryExists(dirpath, policy);
}
//----------------------------------------------------------------------------
bool VFS_FileExists(const FFilename& filename, EExistPolicy policy/* = ExistPolicy::Exists */) {
    return VFS().FileExists(filename, policy);
}
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach) {
    return VFS().EnumerateFiles(dirpath, recursive, foreach);
}
//----------------------------------------------------------------------------
size_t VFS_GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach) {
    return VFS().GlobFiles(dirpath, pattern, recursive, foreach);
}
//----------------------------------------------------------------------------
size_t VFS_MatchFiles(const FDirpath& dirpath, const FWRegexp& re, bool recursive, const TFunction<void(const FFilename&)>& foreach) {
    return VFS().MatchFiles(dirpath, re, recursive, foreach);
}
//----------------------------------------------------------------------------
bool VFS_CreateDirectory(const FDirpath& dirpath) {
    return VFS().CreateDirectory(dirpath);
}
//----------------------------------------------------------------------------
bool VFS_RemoveDirectory(const FDirpath& dirpath, bool force/* = true */) {
    return VFS().RemoveDirectory(dirpath, force);
}
//----------------------------------------------------------------------------
bool VFS_RemoveFile(const FFilename& filename) {
    return VFS().RemoveFile(filename);
}
//----------------------------------------------------------------------------
UStreamReader VFS_OpenReadable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS().OpenReadable(filename, policy);
}
//----------------------------------------------------------------------------
UStreamWriter VFS_OpenWritable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS().OpenWritable(filename, policy);
}
//----------------------------------------------------------------------------
UStreamReadWriter VFS_OpenReadWritable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS().OpenReadWritable(filename, policy);
}
//----------------------------------------------------------------------------
UStreamReader VFS_OpenBinaryReadable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS_OpenReadable(filename, EAccessPolicy(policy|EAccessPolicy::Binary));
}
//----------------------------------------------------------------------------
UStreamWriter VFS_OpenBinaryWritable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS_OpenWritable(filename, EAccessPolicy(policy|EAccessPolicy::Binary));
}
//----------------------------------------------------------------------------
UStreamReader VFS_OpenTextReadable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS_OpenReadable(filename, EAccessPolicy(policy|EAccessPolicy::Text));
}
//----------------------------------------------------------------------------
UStreamWriter VFS_OpenTextWritable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS_OpenWritable(filename, EAccessPolicy(policy|EAccessPolicy::Text));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UStreamWriter VFS_RollFile(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    auto& vfs = VFS();

    FFileStat fstat;
    if (vfs.FileStats(&fstat, filename)) {
        const FDateTime date = fstat.CreatedAt.ToDateTimeUTC();

        wchar_t buffer[256];
        FWFixedSizeTextWriter oss(buffer);
        Format(oss, L"_{0:#4}-{1:#2}-{2:#2}_{3:#2}-{4:#2}-{5:#2}_UTC",
            date.Year, date.Month, date.Day,
            date.Hours, date.Minutes, date.Seconds);

        FFilename logroll = filename;
        logroll.AppendBasename(oss.Written());

        vfs.MoveFile(filename, logroll);
    }

    return vfs.OpenWritable(filename, policy);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<RAWSTORAGE(FileSystem, u8), FRawStorage>::value);
bool VFS_ReadAll(FRawStorage* pcontent, const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return FVirtualFileSystem::ReadAll(filename, *pcontent, policy);
}
//----------------------------------------------------------------------------
bool VFS_WriteAll(const FFilename& filename, const TMemoryView<const u8>& content, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return FVirtualFileSystem::WriteAll(filename, content, policy);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool VFS_FileStats(FFileStat *pstats, const FFilename& filename) {
    return VFS().FileStats(pstats, filename);
}
//----------------------------------------------------------------------------
bool VFS_FileCreatedAt(FTimestamp* ptime, const FFilename& filename) {
    Assert(ptime);

    FFileStat st;
    if (VFS_FileStats(&st, filename)) {
        *ptime = st.CreatedAt;
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool VFS_FileLastModified(FTimestamp* ptime, const FFilename& filename) {
    Assert(ptime);

    FFileStat st;
    if (VFS_FileStats(&st, filename)) {
        *ptime = st.LastModified;
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
