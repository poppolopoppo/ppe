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
bool VFS_DirectoryExists(const FDirpath& dirpath, ExistPolicy::EMode policy/* = ExistPolicy::Exists */) {
    return FVirtualFileSystem::Instance().DirectoryExists(dirpath, policy);
}
//----------------------------------------------------------------------------
bool VFS_FileExists(const FFilename& filename, ExistPolicy::EMode policy/* = ExistPolicy::Exists */) {
    return FVirtualFileSystem::Instance().FileExists(filename, policy);
}
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const std::function<void(const FFilename&)>& foreach) {
    return FVirtualFileSystem::Instance().EnumerateFiles(dirpath, recursive, foreach);
}
//----------------------------------------------------------------------------
bool VFS_TryCreateDirectory(const FDirpath& dirpath) {
    return FVirtualFileSystem::Instance().TryCreateDirectory(dirpath);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenReadable(const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return FVirtualFileSystem::Instance().OpenReadable(filename, policy);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenWritable(const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return FVirtualFileSystem::Instance().OpenWritable(filename, policy);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIOStream> VFS_OpenReadWritable(const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return FVirtualFileSystem::Instance().OpenReadWritable(filename, policy);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenBinaryReadable(const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return VFS_OpenReadable(filename, AccessPolicy::EMode(policy|AccessPolicy::Binary));
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenBinaryWritable(const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return VFS_OpenWritable(filename, AccessPolicy::EMode(policy|AccessPolicy::Binary));
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenTextReadable(const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return VFS_OpenReadable(filename, AccessPolicy::EMode(policy|AccessPolicy::Text));
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenTextWritable(const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return VFS_OpenWritable(filename, AccessPolicy::EMode(policy|AccessPolicy::Text));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<RAWSTORAGE(FileSystem, u8), TRawStorage<u8, ALLOCATOR(FileSystem, u8)> >::value);
bool VFS_ReadAll(TRawStorage<u8, ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return FVirtualFileSystem::ReadAll(filename, *pcontent, policy);
}
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<RAWSTORAGE_THREAD_LOCAL(FileSystem, u8), TRawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> >::value);
bool VFS_ReadAll(TRawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return FVirtualFileSystem::ReadAll(filename, *pcontent, policy);
}
//----------------------------------------------------------------------------
bool VFS_WriteAll(const FFilename& filename, const TMemoryView<const u8>& content, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    return FVirtualFileSystem::WriteAll(filename, content, policy);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool VFS_FileStats(FFileStat *pstats, const FFilename& filename) {
    return FVirtualFileSystem::Instance().FileStats(pstats, filename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
