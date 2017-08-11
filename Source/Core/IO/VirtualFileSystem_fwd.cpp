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
bool VFS_DirectoryExists(const FDirpath& dirpath, EExistPolicy policy/* = ExistPolicy::Exists */) {
    return FVirtualFileSystem::Instance().DirectoryExists(dirpath, policy);
}
//----------------------------------------------------------------------------
bool VFS_FileExists(const FFilename& filename, EExistPolicy policy/* = ExistPolicy::Exists */) {
    return FVirtualFileSystem::Instance().FileExists(filename, policy);
}
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const FDirpath& dirpath, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach) {
    return FVirtualFileSystem::Instance().EnumerateFiles(dirpath, recursive, foreach);
}
//----------------------------------------------------------------------------
size_t VFS_GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach) {
    return FVirtualFileSystem::Instance().GlobFiles(dirpath, pattern, recursive, foreach);
}
//----------------------------------------------------------------------------
bool VFS_CreateDirectory(const FDirpath& dirpath) {
    return FVirtualFileSystem::Instance().CreateDirectory(dirpath);
}
//----------------------------------------------------------------------------
bool VFS_RemoveDirectory(const FDirpath& dirpath) {
    return FVirtualFileSystem::Instance().RemoveDirectory(dirpath);
}
//----------------------------------------------------------------------------
bool VFS_RemoveFile(const FFilename& filename) {
    return FVirtualFileSystem::Instance().RemoveFile(filename);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenReadable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return FVirtualFileSystem::Instance().OpenReadable(filename, policy);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenWritable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return FVirtualFileSystem::Instance().OpenWritable(filename, policy);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIOStream> VFS_OpenReadWritable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return FVirtualFileSystem::Instance().OpenReadWritable(filename, policy);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenBinaryReadable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS_OpenReadable(filename, EAccessPolicy(policy|EAccessPolicy::Binary));
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenBinaryWritable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS_OpenWritable(filename, EAccessPolicy(policy|EAccessPolicy::Binary));
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> VFS_OpenTextReadable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS_OpenReadable(filename, EAccessPolicy(policy|EAccessPolicy::Text));
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemOStream> VFS_OpenTextWritable(const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return VFS_OpenWritable(filename, EAccessPolicy(policy|EAccessPolicy::Text));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<RAWSTORAGE(FileSystem, u8), TRawStorage<u8, ALLOCATOR(FileSystem, u8)> >::value);
bool VFS_ReadAll(TRawStorage<u8, ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
    return FVirtualFileSystem::ReadAll(filename, *pcontent, policy);
}
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same<RAWSTORAGE_THREAD_LOCAL(FileSystem, u8), TRawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> >::value);
bool VFS_ReadAll(TRawStorage<u8, THREAD_LOCAL_ALLOCATOR(FileSystem, u8)> *pcontent, const FFilename& filename, EAccessPolicy policy/* = EAccessPolicy::None */) {
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
    return FVirtualFileSystem::Instance().FileStats(pstats, filename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
