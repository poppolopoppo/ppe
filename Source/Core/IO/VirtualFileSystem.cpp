#include "stdafx.h"

#include "VirtualFileSystem.h"

#include "FileSystem.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "Format.h"
#include "Memory/MemoryStack.h"

#include "VFS/VirtualFileSystemComponent.h"
#include "VFS/VirtualFileSystemNativeComponent.h"
#include "VFS/VirtualFileSystemNode.h"
#include "VFS/VirtualFileSystemTrie.h"

#include <chrono>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define CORE_VIRTUALFILESYSTEM_STACKSIZE Dirpath::MaxDepth
//----------------------------------------------------------------------------
// inverse depth first search : components are pseudo-sorted decreasingly
// by the number of token matching dirpath.
static void MatchingComponents_DepthLast_(
    MemoryStack<VirtualFileSystemComponent *>& components,
    const VirtualFileSystemTrie *trie,
    const Dirpath& dirpath ) {
    MountingPoint mountingPoint;
    const auto dirnames = MALLOCA_VIEW(Dirname, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    const size_t k = dirpath.ExpandPath(mountingPoint, dirnames);

    VirtualFileSystemNode* node = trie->GetNodeIFP(mountingPoint);
    if (!node)
        return;

    for (VirtualFileSystemComponent *component : node->Components())
        components.PushPOD(component);

    for (size_t i = 0; i < k; ++i) {
        for (VirtualFileSystemComponent *component : node->Components())
            components.PushPOD(component);

        if (!(node = node->GetNodeIFP(dirnames[i])) )
            break;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemRoot::VirtualFileSystemRoot() {}
//----------------------------------------------------------------------------
VirtualFileSystemRoot::~VirtualFileSystemRoot() {}
//----------------------------------------------------------------------------
bool VirtualFileSystemRoot::EachComponent(
    const Dirpath& dirpath,
    const std::function<bool(VirtualFileSystemComponent* component)>& foreach) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, dirpath);
    }

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component))
        if (foreach(component))
            return true;

    return false;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemRoot::DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, dirpath);
    }

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable && readable->DirectoryExists(dirpath, policy) )
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemRoot::FileExists(const Filename& filename, ExistPolicy::Mode policy) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, filename.Dirpath());
    }

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable && readable->FileExists(filename, policy) )
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
size_t VirtualFileSystemRoot::EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, dirpath);
    }

    size_t total = 0;

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable)
            total += readable->EnumerateFiles(dirpath, recursive, foreach);
    }

    return total;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemRoot::TryCreateDirectory(const Dirpath& dirpath) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, dirpath);
    }

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component)) {
        IVirtualFileSystemComponentWritable* const writable = component->Writable();
        if (writable && writable->TryCreateDirectory(dirpath) )
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VirtualFileSystemRoot::OpenReadable(const Filename& filename, AccessPolicy::Mode policy) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, filename.Dirpath());
    }

    UniquePtr<IVirtualFileSystemIStream> result;

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable && (result = readable->OpenReadable(filename, policy)) )
            return result;
    }

    return result;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemOStream> VirtualFileSystemRoot::OpenWritable(const Filename& filename, AccessPolicy::Mode policy) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, filename.Dirpath());
    }

    UniquePtr<IVirtualFileSystemOStream> result;

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component)) {
        IVirtualFileSystemComponentWritable* const writable = component->Writable();
        if (writable && (result = writable->OpenWritable(filename, policy)) )
            return result;
    }

    return result;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIOStream> VirtualFileSystemRoot::OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, filename.Dirpath());
    }

    UniquePtr<IVirtualFileSystemIOStream> result;

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component)) {
        IVirtualFileSystemComponentReadWritable* const readWritable = component->ReadWritable();
        if (readWritable && (result = readWritable->OpenReadWritable(filename, policy)) )
            return result;
    }

    return result;
}
//----------------------------------------------------------------------------
WString VirtualFileSystemRoot::Unalias(const Filename& aliased) const {
    MALLOCA_STACK(VirtualFileSystemComponent*, components, CORE_VIRTUALFILESYSTEM_STACKSIZE);
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);
        MatchingComponents_DepthLast_(components, &_trie, aliased.Dirpath());
    }

    VirtualFileSystemComponent* component = nullptr;
    while (components.PopPOD(&component)) {
        return component->Unalias(aliased);
    }

    return WString();
}
//----------------------------------------------------------------------------
void VirtualFileSystemRoot::Clear() {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    _trie.Clear();
}
//----------------------------------------------------------------------------
void VirtualFileSystemRoot::Mount(VirtualFileSystemComponent* component) {
    Assert(component);

    LOG(Information, L"[VFS] Mount component '{0}'", component->Alias());

    std::lock_guard<std::mutex> scopeLock(_barrier);
    _trie.AddComponent(component);
}
//----------------------------------------------------------------------------
void VirtualFileSystemRoot::Unmount(VirtualFileSystemComponent* component) {
    Assert(component);

    LOG(Information, L"[VFS] Unmount component '{0}'", component->Alias());

    std::lock_guard<std::mutex> scopeLock(_barrier);
    _trie.RemoveComponent(component);
}
//----------------------------------------------------------------------------
VirtualFileSystemComponent *VirtualFileSystemRoot::MountNativePath(const Dirpath& alias, const wchar_t *nativepPath) {
    Assert(nativepPath);
    VirtualFileSystemComponent *component = new VirtualFileSystemNativeComponent(alias, nativepPath);
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
VirtualFileSystemComponent *VirtualFileSystemRoot::MountNativePath(const Dirpath& alias, WString&& nativepPath) {
    Assert(nativepPath.size());
    VirtualFileSystemComponent *component = new VirtualFileSystemNativeComponent(alias, std::move(nativepPath));
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
VirtualFileSystemComponent *VirtualFileSystemRoot::MountNativePath(const Dirpath& alias, const WString& nativepPath) {
    Assert(nativepPath.size());
    VirtualFileSystemComponent *component = new VirtualFileSystemNativeComponent(alias, nativepPath);
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
Filename VirtualFileSystemRoot::TemporaryFilename(const wchar_t *prefix, const wchar_t *ext) const {
    Assert(prefix);
    Assert(ext);

    const auto now = std::chrono::system_clock::now();
    const time_t tt = std::chrono::system_clock::to_time_t(now);

    tm local_tm;
    localtime_s(&local_tm, &tt);

    wchar_t buffer[2048];
    const size_t length = Format(buffer, L"Tmp:/{0}_{1:#4}{2:#2}{3:#2}{4:#2}{5:#2}{6:#2}{7}{8}",
        prefix,
        local_tm.tm_year,
        local_tm.tm_mon,
        local_tm.tm_mday,
        local_tm.tm_hour,
        local_tm.tm_min,
        local_tm.tm_sec,
        tt,
        ext );

    return Filename(buffer, length);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VirtualFileSystemStartup::Start() {
    VirtualFileSystem::Create();

    // current process directory
    {
        VirtualFileSystem::Instance().MountNativePath(L"Process:/", CurrentProcess::Instance().Directory());
    }
    // system temporary path
    {
        FileSystem::char_type tmpPath[1024];
        if (!FileSystem::SystemTemporaryDirectory(tmpPath, lengthof(tmpPath)) )
            AssertNotReached();

        VirtualFileSystem::Instance().MountNativePath(L"Tmp:/", tmpPath);
    }
}
//----------------------------------------------------------------------------
void VirtualFileSystemStartup::Shutdown() {
    VirtualFileSystem::Destroy();
}
//----------------------------------------------------------------------------
void VirtualFileSystemStartup::Clear() {
    VirtualFileSystem::Instance().Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
