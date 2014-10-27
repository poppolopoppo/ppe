#pragma once

#include "Core/Core.h"

#include "Core/IO/String.h"
#include "Core/IO/VFS/VirtualFileSystemPolicies.h"
#include "Core/Memory/UniquePtr.h"

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
bool VFS_TryCreateDirectory(const Dirpath& dirpath);
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VFS_OpenReadable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
UniquePtr<IVirtualFileSystemOStream> VFS_OpenWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
UniquePtr<IVirtualFileSystemIOStream> VFS_OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
