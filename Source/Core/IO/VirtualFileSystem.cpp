#include "stdafx.h"

#include "VirtualFileSystem.h"

#include "FileSystem.h"
#include "Format.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "Diagnostic/Logger.h"
#include "Misc/CurrentProcess.h"

#include "VFS/VirtualFileSystemComponent.h"
#include "VFS/VirtualFileSystemNativeComponent.h"
#include "VFS/VirtualFileSystemNode.h"
#include "VFS/VirtualFileSystemTrie.h"

#include <chrono>

#if defined(OS_WINDOWS)
#   include <stdlib.h>
#   include <wchar.h>
#endif

namespace Core {
POOLTAG_DEF(VirtualFileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// inverse depth first search : components are pseudo-sorted decreasingly
// by the number of token matching dirpath.
static void MatchingComponents_DepthLast_(
    Stack<VirtualFileSystemComponent *>& components,
    const VirtualFileSystemTrie *trie,
    const Dirpath& dirpath ) {

    MountingPoint mountingPoint;
    STACKLOCAL_POD_ARRAY(Dirname, dirnames, dirpath.Depth());
    const size_t k = dirpath.ExpandPath(mountingPoint, dirnames);

    VirtualFileSystemNode* node = trie->GetNodeIFP(mountingPoint);
    if (!node)
        return;

    for (const PVirtualFileSystemComponent& component : node->Components())
        components.Push(component.get());

    for (size_t i = 0; i < k; ++i) {
        for (const PVirtualFileSystemComponent& component : node->Components())
            components.Push(component.get());

        if (nullptr == (node = node->GetNodeIFP(dirnames[i])) )
            break;
    }
}
//----------------------------------------------------------------------------
#define STACKLOCAL_MATCHINGCOMPONENTS(_NAME, _DIRPATH) \
    STACKLOCAL_POD_STACK(VirtualFileSystemComponent*, _NAME, (_DIRPATH).Depth()); { \
        READSCOPELOCK(_barrier); \
        MatchingComponents_DepthLast_((_NAME), &_trie, (_DIRPATH)); \
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
    STACKLOCAL_MATCHINGCOMPONENTS(components, dirpath);
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component))
        if (foreach(component))
            return true;

    return false;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemRoot::DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, dirpath);
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable && readable->DirectoryExists(dirpath, policy) )
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemRoot::FileExists(const Filename& filename, ExistPolicy::Mode policy) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, filename.Dirpath());
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable && readable->FileExists(filename, policy) )
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemRoot::FileStats(FileStat* pstat, const Filename& filename) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, filename.Dirpath());
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable && readable->FileStats(pstat, filename) )
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
size_t VirtualFileSystemRoot::EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, dirpath);
    size_t total = 0;
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable)
            total += readable->EnumerateFiles(dirpath, recursive, foreach);
    }

    return total;
}
//----------------------------------------------------------------------------
size_t VirtualFileSystemRoot::GlobFiles(const Dirpath& dirpath, const WStringSlice& pattern, bool recursive, const std::function<void(const Filename&)>& foreach) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, dirpath);
    size_t total = 0;
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable)
            total += readable->GlobFiles(dirpath, pattern, recursive, foreach);
    }

    return total;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemRoot::TryCreateDirectory(const Dirpath& dirpath) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, dirpath);
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentWritable* const writable = component->Writable();
        if (writable && writable->TryCreateDirectory(dirpath) )
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VirtualFileSystemRoot::OpenReadable(const Filename& filename, AccessPolicy::Mode policy) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, filename.Dirpath());
    UniquePtr<IVirtualFileSystemIStream> result;
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentReadable* const readable = component->Readable();
        if (readable && (result = readable->OpenReadable(filename, policy)) )
            return result;
    }

    return result;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemOStream> VirtualFileSystemRoot::OpenWritable(const Filename& filename, AccessPolicy::Mode policy) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, filename.Dirpath());
    UniquePtr<IVirtualFileSystemOStream> result;
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentWritable* const writable = component->Writable();
        if (writable && (result = writable->OpenWritable(filename, policy)) )
            return result;
    }

    return result;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIOStream> VirtualFileSystemRoot::OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, filename.Dirpath());
    UniquePtr<IVirtualFileSystemIOStream> result;
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        IVirtualFileSystemComponentReadWritable* const readWritable = component->ReadWritable();
        if (readWritable && (result = readWritable->OpenReadWritable(filename, policy)) )
            return result;
    }

    return result;
}
//----------------------------------------------------------------------------
WString VirtualFileSystemRoot::Unalias(const Filename& aliased) const {
    STACKLOCAL_MATCHINGCOMPONENTS(components, aliased.Dirpath());
    VirtualFileSystemComponent* component = nullptr;
    while (components.Pop(&component)) {
        return component->Unalias(aliased);
    }

    return WString();
}
//----------------------------------------------------------------------------
void VirtualFileSystemRoot::Clear() {
    WRITESCOPELOCK(_barrier);
    _trie.Clear();
}
//----------------------------------------------------------------------------
void VirtualFileSystemRoot::Mount(VirtualFileSystemComponent* component) {
    Assert(component);

    LOG(Info, L"[VFS] Mount component '{0}'", component->Alias());

    WRITESCOPELOCK(_barrier);
    _trie.AddComponent(component);
}
//----------------------------------------------------------------------------
void VirtualFileSystemRoot::Unmount(VirtualFileSystemComponent* component) {
    Assert(component);

    LOG(Info, L"[VFS] Unmount component '{0}'", component->Alias());

    WRITESCOPELOCK(_barrier);
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
bool VirtualFileSystemRoot::WriteAll(const Filename& filename, const MemoryView<const u8>& storage, AccessPolicy::Mode policy /* = AccessPolicy::None */) {
    const UniquePtr<IVirtualFileSystemOStream> ostream = OpenWritable(filename, policy);
    if (ostream) {
        ostream->Write(storage.Pointer(), storage.SizeInBytes());
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VirtualFileSystemStartup::Start() {
    POOLTAG(VirtualFileSystem)::Start();
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

        VirtualFileSystem::Instance().MountNativePath(L"Temp:/", tmpPath);
    }
    // user profile path
    {
        const wchar_t* userPath = nullptr;
#if defined(OS_WINDOWS)
        userPath = _wgetenv(L"USERPROFILE");
#elif defined(OS_LINUX)
        const char* userPathA = std::getenv("HOME");
        WString userPathW = ToWString(userPath);
        userPath = userPathW.c_str();
#else
#   error "unsupported platform"
#endif
        AssertRelease(userPath);
        VirtualFileSystem::Instance().MountNativePath(L"User:/", userPath);
    }
}
//----------------------------------------------------------------------------
void VirtualFileSystemStartup::Shutdown() {
    VirtualFileSystem::Destroy();
    POOLTAG(VirtualFileSystem)::Shutdown();
}
//----------------------------------------------------------------------------
void VirtualFileSystemStartup::Clear() {
    VirtualFileSystem::Instance().Clear();
    POOLTAG(VirtualFileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
