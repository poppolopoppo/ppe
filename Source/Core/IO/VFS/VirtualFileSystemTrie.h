#pragma once

#include "Core/Core.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/IO/FS/MountingPoint.h"
#include "Core/IO/VirtualFileSystem_fwd.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Thread/ReadWriteLock.h"

#include <functional>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Dirpath;
class FileStat;
FWD_REFPTR(VirtualFileSystemComponent);
//----------------------------------------------------------------------------
class VirtualFileSystemTrie {
public:
    typedef ASSOCIATIVE_VECTORINSITU(FileSystem, MountingPoint, PVirtualFileSystemComponent, 8) nodes_type;

    VirtualFileSystemTrie();
    ~VirtualFileSystemTrie();

    VirtualFileSystemTrie(const VirtualFileSystemTrie& other) = delete;
    VirtualFileSystemTrie& operator =(const VirtualFileSystemTrie& other) = delete;

    bool DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy = ExistPolicy::Exists) const;
    bool FileExists(const Filename& filename, ExistPolicy::Mode policy = ExistPolicy::Exists) const;
    bool FileStats(FileStat* pstat, const Filename& filename) const;

    size_t EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) const;
    size_t GlobFiles(const Dirpath& dirpath, const WStringSlice& pattern, bool recursive, const std::function<void(const Filename&)>& foreach) const;

    bool TryCreateDirectory(const Dirpath& dirpath) const;

    UniquePtr<IVirtualFileSystemIStream> OpenReadable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None) const;
    UniquePtr<IVirtualFileSystemOStream> OpenWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None) const;
    UniquePtr<IVirtualFileSystemIOStream> OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None) const;

    WString Unalias(const Filename& aliased) const;

    void Clear();

    void Mount(VirtualFileSystemComponent* component);
    void Unmount(VirtualFileSystemComponent* component);

    VirtualFileSystemComponent* MountNativePath(const Dirpath& alias, const wchar_t *nativepPath);
    VirtualFileSystemComponent* MountNativePath(const Dirpath& alias, WString&& nativepPath);
    VirtualFileSystemComponent* MountNativePath(const Dirpath& alias, const WString& nativepPath);

private:
    ReadWriteLock _barrier;
    nodes_type _nodes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
