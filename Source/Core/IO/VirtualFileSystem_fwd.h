#pragma once

#include "Core/Core.h"

#include "Core/IO/FileSystem_fwd.h"
#include "Core/IO/VFS/VirtualFileSystemPolicies.h"
#include "Core/Memory/UniquePtr.h"

#include <functional>

namespace Core {
class IVirtualFileSystemIStream;
class IVirtualFileSystemIOStream;
class IVirtualFileSystemOStream;

class FFileStat;
template <typename T>
class TMemoryView;
template <typename T, typename _Allocator>
class TRawStorage;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool VFS_DirectoryExists(const FDirpath& dirpath, ExistPolicy::EMode policy = ExistPolicy::Exists);
bool VFS_FileExists(const FFilename& filename, ExistPolicy::EMode policy = ExistPolicy::Exists);
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const std::function<void(const FFilename&)>& foreach);
//----------------------------------------------------------------------------
bool VFS_TryCreateDirectory(const FDirpath& dirpath);
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenReadable(const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenWritable(const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
TUniquePtr<IVirtualFileSystemIOStream> VFS_OpenReadWritable(const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenBinaryReadable(const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenBinaryWritable(const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenTextReadable(const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenTextWritable(const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
bool VFS_ReadAll(TRawStorage<u8, ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
bool VFS_ReadAll(TRawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, AccessPolicy::EMode policy = AccessPolicy::None);
bool VFS_WriteAll(const FFilename& filename, const TMemoryView<const u8>& content, AccessPolicy::EMode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
bool VFS_FileStats(FFileStat *pstats, const FFilename& filename);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
