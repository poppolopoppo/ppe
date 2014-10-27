#pragma once

#include "Core/Core.h"

#include "Core/IO/VFS/VirtualFileSystemComponent.h"
#include "Core/IO/VFS/VirtualFileSystemTrie.h"

#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Singleton.h"

#include <mutex>

namespace Core {
template <typename T, typename _Allocator>
class RawStorage;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IVirtualFileSystemIStream;
class IVirtualFileSystemOStream;
class IVirtualFileSystemIOStream;
//----------------------------------------------------------------------------
class VirtualFileSystemRoot {
public:
    VirtualFileSystemRoot();
    ~VirtualFileSystemRoot();

    bool EachComponent(const Dirpath& dirpath, const std::function<bool(VirtualFileSystemComponent* component)>& foreach) const;

    bool DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy = ExistPolicy::Exists) const;
    bool FileExists(const Filename& filename, ExistPolicy::Mode policy = ExistPolicy::Exists) const;

    size_t EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) const;

    bool TryCreateDirectory(const Dirpath& dirpath) const;

    UniquePtr<IVirtualFileSystemIStream> OpenReadable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None) const;
    UniquePtr<IVirtualFileSystemOStream> OpenWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None) const;
    UniquePtr<IVirtualFileSystemIOStream> OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy = AccessPolicy::None) const;

    WString Unalias(const Filename& aliased) const;

    void Clear();

    void Mount(VirtualFileSystemComponent* component);
    void Unmount(VirtualFileSystemComponent* component);

    VirtualFileSystemComponent *MountNativePath(const Dirpath& alias, const wchar_t *nativepPath);
    VirtualFileSystemComponent *MountNativePath(const Dirpath& alias, WString&& nativepPath);
    VirtualFileSystemComponent *MountNativePath(const Dirpath& alias, const WString& nativepPath);

    Filename TemporaryFilename(const wchar_t *prefix, const wchar_t *ext) const;

    template <typename T, typename _Allocator>
    void ReadAll_DiscardData(const Filename& filename, RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy = AccessPolicy::None);

private:
    mutable std::mutex _barrier;
    VirtualFileSystemTrie _trie;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VirtualFileSystem : Meta::Singleton<VirtualFileSystemRoot, VirtualFileSystem> {
public:
    typedef Meta::Singleton<VirtualFileSystemRoot, VirtualFileSystem> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VirtualFileSystemStartup {
public:
    static void Start();
    static void Shutdown();

    static void Clear();

    VirtualFileSystemStartup() { Start(); }
    ~VirtualFileSystemStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/VirtualFileSystem-inl.h"
