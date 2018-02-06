#pragma once

#include "Core/Core.h"

#include "Core/IO/FileSystem_fwd.h"
#include "Core/IO/StreamPolicies.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Function.h"

#include <functional>

namespace Core {

class FFileStat;
template <typename T>
class TMemoryView;
template <typename T, typename _Allocator>
class TRawStorage;

FWD_INTEFARCE_UNIQUEPTR(StreamReader);
FWD_INTEFARCE_UNIQUEPTR(StreamWriter);
FWD_INTEFARCE_UNIQUEPTR(StreamReadWriter);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API bool VFS_DirectoryExists(const FDirpath& dirpath, EExistPolicy policy = EExistPolicy::Exists);
CORE_API bool VFS_FileExists(const FFilename& filename, EExistPolicy policy = EExistPolicy::Exists);
CORE_API bool VFS_FileStats(FFileStat* pstat, const FFilename& filename);
//----------------------------------------------------------------------------
CORE_API size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach);
CORE_API size_t VFS_GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach);
//----------------------------------------------------------------------------
CORE_API bool VFS_CreateDirectory(const FDirpath& dirpath);
CORE_API bool VFS_RemoveDirectory(const FDirpath& dirpath);
CORE_API bool VFS_RemoveFile(const FFilename& filename);
//----------------------------------------------------------------------------
CORE_API UStreamReader VFS_OpenReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
CORE_API UStreamWriter VFS_OpenWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
CORE_API UStreamReadWriter VFS_OpenReadWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
CORE_API UStreamReader VFS_OpenBinaryReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
CORE_API UStreamWriter VFS_OpenBinaryWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
CORE_API UStreamReader VFS_OpenTextReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
CORE_API UStreamWriter VFS_OpenTextWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
CORE_API UStreamWriter VFS_RollFile(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
CORE_API bool VFS_ReadAll(TRawStorage<u8, ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
CORE_API bool VFS_ReadAll(TRawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
CORE_API bool VFS_WriteAll(const FFilename& filename, const TMemoryView<const u8>& content, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
