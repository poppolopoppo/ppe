#include "stdafx.h"

#include "VirtualFileSystem_fwd.h"

#include "VirtualFileSystem.h"

#include "Container/RawStorage.h"
#include "FS/Dirpath.h"
#include "FS/Filename.h"
#include "FS/FileStat.h"
#include "VFS/VirtualFileSystemStream.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool VFS_DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy/* = ExistPolicy::Exists */) {
    return VirtualFileSystem::Instance().DirectoryExists(dirpath, policy);
}
//----------------------------------------------------------------------------
bool VFS_FileExists(const Filename& filename, ExistPolicy::Mode policy/* = ExistPolicy::Exists */) {
    return VirtualFileSystem::Instance().FileExists(filename, policy);
}
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) {
    return VirtualFileSystem::Instance().EnumerateFiles(dirpath, recursive, foreach);
}
//----------------------------------------------------------------------------
bool VFS_TryCreateDirectory(const Dirpath& dirpath) {
    return VirtualFileSystem::Instance().TryCreateDirectory(dirpath);
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VFS_OpenReadable(const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VirtualFileSystem::Instance().OpenReadable(filename, policy);
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemOStream> VFS_OpenWritable(const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VirtualFileSystem::Instance().OpenWritable(filename, policy);
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIOStream> VFS_OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VirtualFileSystem::Instance().OpenReadWritable(filename, policy);
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VFS_OpenBinaryReadable(const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VFS_OpenReadable(filename, AccessPolicy::Mode(policy|AccessPolicy::Binary));
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemOStream> VFS_OpenBinaryWritable(const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VFS_OpenWritable(filename, AccessPolicy::Mode(policy|AccessPolicy::Binary));
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VFS_OpenTextReadable(const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VFS_OpenReadable(filename, AccessPolicy::Mode(policy|AccessPolicy::Text));
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemOStream> VFS_OpenTextWritable(const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VFS_OpenWritable(filename, AccessPolicy::Mode(policy|AccessPolicy::Text));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<RAWSTORAGE(FileSystem, u8), RawStorage<u8, ALLOCATOR(FileSystem, u8)> >::value);
bool VFS_ReadAll(RawStorage<u8, ALLOCATOR(FileSystem, u8)> *pcontent, const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VirtualFileSystem::ReadAll(filename, *pcontent, policy);
}
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<RAWSTORAGE_THREAD_LOCAL(FileSystem, u8), RawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> >::value);
bool VFS_ReadAll(RawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> *pcontent, const Filename& filename, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VirtualFileSystem::ReadAll(filename, *pcontent, policy);
}
//----------------------------------------------------------------------------
bool VFS_WriteAll(const Filename& filename, const MemoryView<const u8>& content, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    return VirtualFileSystem::WriteAll(filename, content, policy);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool VFS_FileStats(FileStat *pstats, const Filename& filename) {
    return VirtualFileSystem::Instance().FileStats(pstats, filename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
