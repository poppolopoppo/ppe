#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_VFS
#   define PPE_VFS_API DLL_EXPORT
#else
#   define PPE_VFS_API DLL_IMPORT
#endif

#include "Allocator/Allocation.h"
#include "Container/RawStorage_fwd.h"
#include "IO/FileSystem_fwd.h"
#include "IO/regexp.h"
#include "IO/String_fwd.h"
#include "IO/StreamPolicies.h"
#include "IO/StreamProvider.h"
#include "Memory/SharedBuffer_fwd.h"
#include "Memory/UniquePtr.h"
#include "Misc/Function_fwd.h"

namespace PPE {

class FTimestamp;
struct FGenericPlatformFileStat;
using FFileStat = FGenericPlatformFileStat;

template <typename T>
class TMemoryView;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FRawStorage = TRawStorage<u8, ALLOCATOR(FileSystem)>;
//----------------------------------------------------------------------------
PPE_VFS_API class FVirtualFileSystemTrie& VFS();
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API FWString VFS_Unalias(const FDirpath& dirpath);
NODISCARD PPE_VFS_API FWString VFS_Unalias(const FFilename& filename);
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API bool VFS_DirectoryExists(const FDirpath& dirpath, EExistPolicy policy = EExistPolicy::Exists);
NODISCARD PPE_VFS_API bool VFS_FileExists(const FFilename& filename, EExistPolicy policy = EExistPolicy::Exists);
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API bool VFS_FileStats(FFileStat* pstat, const FFilename& filename);
NODISCARD PPE_VFS_API bool VFS_FileCreatedAt(FTimestamp* ptime, const FFilename& filename);
NODISCARD PPE_VFS_API bool VFS_FileLastModified(FTimestamp* ptime, const FFilename& filename);
//----------------------------------------------------------------------------
PPE_VFS_API size_t VFS_EnumerateMountingPoints(const TFunction<void(const FMountingPoint&)>& foreach);
PPE_VFS_API size_t VFS_EnumerateDir(const FDirpath& dirpath, bool recursive, const TFunction<void(const FDirpath&)>& onDirectory, const TFunction<void(const FFilename&)>& onFile);
PPE_VFS_API size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach);
PPE_VFS_API size_t VFS_GlobFiles(const FDirpath& dirpath, FWStringLiteral pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach);
PPE_VFS_API size_t VFS_GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach);
PPE_VFS_API size_t VFS_MatchFiles(const FDirpath& dirpath, const FWRegexp& re, bool recursive, const TFunction<void(const FFilename&)>& foreach);
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API bool VFS_CreateDirectory(const FDirpath& dirpath);
NODISCARD PPE_VFS_API bool VFS_RemoveDirectory(const FDirpath& dirpath, bool force = true);
NODISCARD PPE_VFS_API bool VFS_RemoveFile(const FFilename& filename);
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API UStreamReader VFS_OpenReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
NODISCARD PPE_VFS_API UStreamWriter VFS_OpenWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
NODISCARD PPE_VFS_API UStreamReadWriter VFS_OpenReadWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API UStreamReader VFS_OpenBinaryReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
NODISCARD PPE_VFS_API UStreamWriter VFS_OpenBinaryWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API UStreamReader VFS_OpenTextReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
NODISCARD PPE_VFS_API UStreamWriter VFS_OpenTextWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API UStreamWriter VFS_RollFile(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
NODISCARD PPE_VFS_API FUniqueBuffer VFS_ReadAll(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
NODISCARD PPE_VFS_API bool VFS_ReadAnsiString(FString* pstring, const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
NODISCARD PPE_VFS_API bool VFS_WriteAll(const FFilename& filename, const TMemoryView<const u8>& content, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
