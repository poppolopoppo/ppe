#include "stdafx.h"

#include "VirtualFileSystemTrie.h"

#include "Diagnostic/Logger.h"
#include "IO/FileSystem.h"
#include "Memory/MemoryView.h"
#include "Memory/UniqueView.h"

#include "VirtualFileSystemComponent.h"
#include "VirtualFileSystemNativeComponent.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static VirtualFileSystemComponent* VFSComponent_(
    const MountingPoint& mountingPoint,
    const VirtualFileSystemTrie::nodes_type& nodes ) {
    Assert(false == mountingPoint.empty());
    const auto it = nodes.Find(mountingPoint);
    if (nodes.end() == it)
        return nullptr;
    Assert(it->second);
    return it->second.get();
}
//----------------------------------------------------------------------------
static IVirtualFileSystemComponentReadable* ReadableComponent_(
    const MountingPoint& mountingPoint,
    const VirtualFileSystemTrie::nodes_type& nodes ) {
    VirtualFileSystemComponent* const p = VFSComponent_(mountingPoint, nodes);
    return (p ? p->Readable() : nullptr);
}
//----------------------------------------------------------------------------
static IVirtualFileSystemComponentWritable* WritableComponent_(
    const MountingPoint& mountingPoint,
    const VirtualFileSystemTrie::nodes_type& nodes ) {
    VirtualFileSystemComponent* const p = VFSComponent_(mountingPoint, nodes);
    return (p ? p->Writable() : nullptr);
}
//----------------------------------------------------------------------------
static IVirtualFileSystemComponentReadWritable* ReadWritableComponent_(
    const MountingPoint& mountingPoint,
    const VirtualFileSystemTrie::nodes_type& nodes ) {
    VirtualFileSystemComponent* const p = VFSComponent_(mountingPoint, nodes);
    return (p ? p->ReadWritable() : nullptr);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemTrie::VirtualFileSystemTrie() {}
//----------------------------------------------------------------------------
VirtualFileSystemTrie::~VirtualFileSystemTrie() { Clear(); }
//----------------------------------------------------------------------------
bool VirtualFileSystemTrie::DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(dirpath.MountingPoint(), _nodes);
    return (readable)
        ? readable->DirectoryExists(dirpath, policy)
        : false;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemTrie::FileExists(const Filename& filename, ExistPolicy::Mode policy) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(filename.MountingPoint(), _nodes);
    return (readable)
        ? readable->FileExists(filename, policy)
        : false;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemTrie::FileStats(FileStat* pstat, const Filename& filename) const {
    Assert(pstat);
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(filename.MountingPoint(), _nodes);
    return (readable)
        ? readable->FileStats(pstat, filename)
        : false;
}
//----------------------------------------------------------------------------
size_t VirtualFileSystemTrie::EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(dirpath.MountingPoint(), _nodes);
    return (readable)
        ? readable->EnumerateFiles(dirpath, recursive, foreach)
        : 0;
}
//----------------------------------------------------------------------------
size_t VirtualFileSystemTrie::GlobFiles(const Dirpath& dirpath, const WStringView& pattern, bool recursive, const std::function<void(const Filename&)>& foreach) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(dirpath.MountingPoint(), _nodes);
    return (readable)
        ? readable->GlobFiles(dirpath, pattern, recursive, foreach)
        : 0;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemTrie::TryCreateDirectory(const Dirpath& dirpath) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentWritable* const writable = WritableComponent_(dirpath.MountingPoint(), _nodes);
    return (writable)
        ? writable->TryCreateDirectory(dirpath)
        : false;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VirtualFileSystemTrie::OpenReadable(const Filename& filename, AccessPolicy::Mode policy) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(filename.MountingPoint(), _nodes);
    UniquePtr<IVirtualFileSystemIStream> result;
    if (readable)
        result = readable->OpenReadable(filename, policy);
    return result;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemOStream> VirtualFileSystemTrie::OpenWritable(const Filename& filename, AccessPolicy::Mode policy) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentWritable* const writable = WritableComponent_(filename.MountingPoint(), _nodes);
    UniquePtr<IVirtualFileSystemOStream> result;
    if (writable)
        result = writable->OpenWritable(filename, policy);
    return result;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIOStream> VirtualFileSystemTrie::OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadWritable* const readWritable = ReadWritableComponent_(filename.MountingPoint(), _nodes);
    UniquePtr<IVirtualFileSystemIOStream> result;
    if (readWritable)
        result = readWritable->OpenReadWritable(filename, policy);
    return result;
}
//----------------------------------------------------------------------------
WString VirtualFileSystemTrie::Unalias(const Filename& aliased) const {
    READSCOPELOCK(_barrier);
    VirtualFileSystemComponent* const component = VFSComponent_(aliased.MountingPoint(), _nodes);
    WString result;
    if (component)
        result = component->Unalias(aliased);
    return result;
}
//----------------------------------------------------------------------------
void VirtualFileSystemTrie::Clear() {
    WRITESCOPELOCK(_barrier);
    for (Pair<MountingPoint, PVirtualFileSystemComponent>& it : _nodes)
        RemoveRef_AssertReachZero(it.second);

    _nodes.Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void VirtualFileSystemTrie::Mount(VirtualFileSystemComponent* component) {
    Assert(component);
    Assert(component->Alias().HasMountingPoint());
    LOG(Info, L"[VFS] Mount component '{0}'", component->Alias());
    WRITESCOPELOCK(_barrier);
    _nodes.Insert_AssertUnique(component->Alias().MountingPoint(), component);
}
//----------------------------------------------------------------------------
void VirtualFileSystemTrie::Unmount(VirtualFileSystemComponent* component) {
    Assert(component);
    Assert(component->Alias().HasMountingPoint());
    LOG(Info, L"[VFS] Unmount component '{0}'", component->Alias());
    WRITESCOPELOCK(_barrier);
    _nodes.Remove_AssertExists(component->Alias().MountingPoint(), component);
}
//----------------------------------------------------------------------------
VirtualFileSystemComponent* VirtualFileSystemTrie::MountNativePath(const Dirpath& alias, const wchar_t *nativepPath) {
    Assert(nativepPath);
    const SVirtualFileSystemComponent component = new VirtualFileSystemNativeComponent(alias, nativepPath);
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
VirtualFileSystemComponent *VirtualFileSystemTrie::MountNativePath(const Dirpath& alias, WString&& nativepPath) {
    Assert(nativepPath.size());
    VirtualFileSystemComponent *component = new VirtualFileSystemNativeComponent(alias, std::move(nativepPath));
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
VirtualFileSystemComponent *VirtualFileSystemTrie::MountNativePath(const Dirpath& alias, const WString& nativepPath) {
    Assert(nativepPath.size());
    VirtualFileSystemComponent *component = new VirtualFileSystemNativeComponent(alias, nativepPath);
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
