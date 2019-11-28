#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_VFS
#   define PPE_VFS_API DLL_EXPORT
#else
#   define PPE_VFS_API DLL_IMPORT
#endif

#include "Allocator/Allocation.h"
#include "IO/FileSystem_fwd.h"
#include "IO/Regexp.h"
#include "IO/StreamPolicies.h"
#include "IO/StreamProvider.h"
#include "Memory/UniquePtr.h"
#include "Misc/Function_fwd.h"

namespace PPE {

class FTimestamp;
struct FGenericPlatformFileStat;
using FFileStat = FGenericPlatformFileStat;

template <typename T>
class TMemoryView;
template <typename T, typename _Allocator>
class TRawStorage;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FRawStorage = TRawStorage<u8, ALLOCATOR(FileSystem)>;
//----------------------------------------------------------------------------
PPE_VFS_API class FVirtualFileSystemTrie& VFS();
//----------------------------------------------------------------------------
PPE_VFS_API bool VFS_DirectoryExists(const FDirpath& dirpath, EExistPolicy policy = EExistPolicy::Exists);
PPE_VFS_API bool VFS_FileExists(const FFilename& filename, EExistPolicy policy = EExistPolicy::Exists);
//----------------------------------------------------------------------------
PPE_VFS_API bool VFS_FileStats(FFileStat* pstat, const FFilename& filename);
PPE_VFS_API bool VFS_FileCreatedAt(FTimestamp* ptime, const FFilename& filename);
PPE_VFS_API bool VFS_FileLastModified(FTimestamp* ptime, const FFilename& filename);
//----------------------------------------------------------------------------
PPE_VFS_API size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach);
PPE_VFS_API size_t VFS_GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach);
PPE_VFS_API size_t VFS_MatchFiles(const FDirpath& dirpath, const FWRegexp& re, bool recursive, const TFunction<void(const FFilename&)>& foreach);
//----------------------------------------------------------------------------
PPE_VFS_API bool VFS_CreateDirectory(const FDirpath& dirpath);
PPE_VFS_API bool VFS_RemoveDirectory(const FDirpath& dirpath, bool force = true);
PPE_VFS_API bool VFS_RemoveFile(const FFilename& filename);
//----------------------------------------------------------------------------
PPE_VFS_API UStreamReader VFS_OpenReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
PPE_VFS_API UStreamWriter VFS_OpenWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
PPE_VFS_API UStreamReadWriter VFS_OpenReadWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
PPE_VFS_API UStreamReader VFS_OpenBinaryReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
PPE_VFS_API UStreamWriter VFS_OpenBinaryWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
PPE_VFS_API UStreamReader VFS_OpenTextReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
PPE_VFS_API UStreamWriter VFS_OpenTextWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
PPE_VFS_API UStreamWriter VFS_RollFile(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
PPE_VFS_API bool VFS_ReadAll(FRawStorage* pcontent, const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
PPE_VFS_API bool VFS_WriteAll(const FFilename& filename, const TMemoryView<const u8>& content, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
