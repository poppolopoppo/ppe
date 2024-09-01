// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VirtualFileSystem_fwd.h"

#include "VFSModule.h"
#include "VirtualFileSystem.h"

#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/FileStream.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "HAL/PlatformFile.h"
#include "Memory/SharedBuffer.h"
#include "Misc/Function.h"
#include "Time/Timestamp.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemTrie& VFS() {
    return FVirtualFileSystem::Get();
}
//----------------------------------------------------------------------------
FWString VFS_Unalias(const FDirpath& dirpath) {
    return VFS().Unalias(dirpath);
}
//----------------------------------------------------------------------------
FWString VFS_Unalias(const FFilename& filename) {
    return VFS().Unalias(filename);
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
size_t VFS_EnumerateMountingPoints(const TFunctionRef<void(const FMountingPoint&)>& foreach) {
    return VFS().EnumerateMountingPoints(foreach);
}
//----------------------------------------------------------------------------
size_t VFS_EnumerateDir(const FDirpath& dirpath, bool recursive, const TFunctionRef<void(const FDirpath&)>& onDirectory, const TFunctionRef<void(const FFilename&)>& onFile) {
    return VFS().EnumerateDir(dirpath, recursive, onDirectory, onFile);
}
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunctionRef<void(const FFilename&)>& foreach) {
    return VFS().EnumerateFiles(dirpath, recursive, foreach);
}
//----------------------------------------------------------------------------
size_t VFS_GlobFiles(const FDirpath& dirpath, FWStringLiteral pattern, bool recursive, const TFunctionRef<void(const FFilename&)>& foreach) {
    return VFS().GlobFiles(dirpath, pattern, recursive, foreach);
}
//----------------------------------------------------------------------------
size_t VFS_GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunctionRef<void(const FFilename&)>& foreach) {
    return VFS().GlobFiles(dirpath, pattern, recursive, foreach);
}
//----------------------------------------------------------------------------
size_t VFS_MatchFiles(const FDirpath& dirpath, const FWRegexp& re, bool recursive, const TFunctionRef<void(const FFilename&)>& foreach) {
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
    return VFS_OpenWritable(filename, policy|EAccessPolicy::Create|EAccessPolicy::Roll);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<RAWSTORAGE(FileSystem, u8), FRawStorage>::value);
//----------------------------------------------------------------------------
FUniqueBuffer VFS_ReadAll(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return FVirtualFileSystem::ReadAll(filename, policy);
}
//----------------------------------------------------------------------------
bool VFS_ReadAnsiString(FString* pstring, const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    const UStreamReader reader = FVirtualFileSystem::Get().OpenReadable(filename, policy + EAccessPolicy::Binary);
    PPE_LOG_CHECK(VFS, reader.valid());

    FStringBuilder sb;
    sb.reserve(reader->SizeInBytes() + sizeof(char)/* null terminated */);

    PPE_LOG_CHECK(VFS, reader->ReadView(sb.AppendUninitialized(reader->SizeInBytes())));

    sb.ToString(*pstring);
    return true;
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
