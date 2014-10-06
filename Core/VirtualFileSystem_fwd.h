#pragma once

#include "Core.h"

#include "String.h"
#include "UniquePtr.h"
#include "VirtualFileSystemPolicies.h"

#include <functional>

namespace Core {
class Dirpath;
class Filename;

class IVirtualFileSystemIStream;
class IVirtualFileSystemIOStream;
class IVirtualFileSystemOStream;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool VFS_DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy = ExistPolicy::Exists);
bool VFS_FileExists(const Filename& filename, ExistPolicy::Mode policy = ExistPolicy::Exists);
//----------------------------------------------------------------------------
size_t VFS_EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach);
//----------------------------------------------------------------------------
bool VFS_CreateDirectory(const Dirpath& dirpath);
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VFS_OpenReadable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
UniquePtr<IVirtualFileSystemOStream> VFS_OpenWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
UniquePtr<IVirtualFileSystemIOStream> VFS_OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
