#include "stdafx.h"

#include "VirtualFileSystem_fwd.h"

#include "FS/Dirpath.h"
#include "FS/Filename.h"
#include "VirtualFileSystem.h"

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
