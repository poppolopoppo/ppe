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

class FileStat;
template <typename T>
class MemoryView;
template <typename T, typename _Allocator>
class RawStorage;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool VFS_DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy = ExistPolicy::Exists);
bool VFS_FileExists(const Filename& filename, ExistPolicy::Mode policy = ExistPolicy::Exists);
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach);
//----------------------------------------------------------------------------
bool VFS_TryCreateDirectory(const Dirpath& dirpath);
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VFS_OpenReadable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
UniquePtr<IVirtualFileSystemOStream> VFS_OpenWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
UniquePtr<IVirtualFileSystemIOStream> VFS_OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VFS_OpenBinaryReadable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
UniquePtr<IVirtualFileSystemOStream> VFS_OpenBinaryWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VFS_OpenTextReadable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
UniquePtr<IVirtualFileSystemOStream> VFS_OpenTextWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
bool VFS_ReadAll(RawStorage<u8, ALLOCATOR(FileSystem, u8)> *pcontent, const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
bool VFS_ReadAll(RawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> *pcontent, const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
bool VFS_WriteAll(const Filename& filename, const MemoryView<const u8>& content, AccessPolicy::Mode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
bool VFS_FileStats(FileStat *pstats, const Filename& filename);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
