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
bool VFS_DirectoryExists(const FDirpath& dirpath, EExistPolicy policy = EExistPolicy::Exists);
bool VFS_FileExists(const FFilename& filename, EExistPolicy policy = EExistPolicy::Exists);
bool VFS_FileStats(FFileStat* pstat, const FFilename& filename);
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach);
size_t VFS_GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach);
//----------------------------------------------------------------------------
bool VFS_CreateDirectory(const FDirpath& dirpath);
bool VFS_RemoveDirectory(const FDirpath& dirpath);
bool VFS_RemoveFile(const FFilename& filename);
//----------------------------------------------------------------------------
UStreamReader VFS_OpenReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
UStreamWriter VFS_OpenWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
UStreamReadWriter VFS_OpenReadWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
UStreamReader VFS_OpenBinaryReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
UStreamWriter VFS_OpenBinaryWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
UStreamReader VFS_OpenTextReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
UStreamWriter VFS_OpenTextWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
bool VFS_ReadAll(TRawStorage<u8, ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
bool VFS_ReadAll(TRawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
bool VFS_WriteAll(const FFilename& filename, const TMemoryView<const u8>& content, EAccessPolicy policy = EAccessPolicy::None);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
