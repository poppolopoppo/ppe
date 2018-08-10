#include "stdafx.h"

#include "VirtualFileSystemTrie.h"

#include "Diagnostic/Logger.h"
#include "IO/FileSystem.h"
#include "Memory/MemoryView.h"
#include "Memory/UniqueView.h"

#include "VirtualFileSystemComponent.h"
#include "VirtualFileSystemNativeComponent.h"

namespace PPE {
LOG_CATEGORY(PPE_VFS_API, VFS)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FVirtualFileSystemComponent* VFSComponent_(
    const FMountingPoint& mountingPoint,
    const FVirtualFileSystemTrie::nodes_type& nodes ) {
    Assert(false == mountingPoint.empty());
    const auto it = nodes.Find(mountingPoint);
    if (nodes.end() == it)
        return nullptr;
    Assert(it->second);
    return it->second.get();
}
//----------------------------------------------------------------------------
static IVirtualFileSystemComponentReadable* ReadableComponent_(
    const FMountingPoint& mountingPoint,
    const FVirtualFileSystemTrie::nodes_type& nodes ) {
    FVirtualFileSystemComponent* const p = VFSComponent_(mountingPoint, nodes);
    return (p ? p->Readable() : nullptr);
}
//----------------------------------------------------------------------------
static IVirtualFileSystemComponentWritable* WritableComponent_(
    const FMountingPoint& mountingPoint,
    const FVirtualFileSystemTrie::nodes_type& nodes ) {
    FVirtualFileSystemComponent* const p = VFSComponent_(mountingPoint, nodes);
    return (p ? p->Writable() : nullptr);
}
//----------------------------------------------------------------------------
static IVirtualFileSystemComponentReadWritable* ReadWritableComponent_(
    const FMountingPoint& mountingPoint,
    const FVirtualFileSystemTrie::nodes_type& nodes ) {
    FVirtualFileSystemComponent* const p = VFSComponent_(mountingPoint, nodes);
    return (p ? p->ReadWritable() : nullptr);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemTrie::FVirtualFileSystemTrie() {}
//----------------------------------------------------------------------------
FVirtualFileSystemTrie::~FVirtualFileSystemTrie() { Clear(); }
//----------------------------------------------------------------------------
bool FVirtualFileSystemTrie::DirectoryExists(const FDirpath& dirpath, EExistPolicy policy) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(dirpath.MountingPoint(), _nodes);
    return (readable)
        ? readable->DirectoryExists(dirpath, policy)
        : false;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemTrie::FileExists(const FFilename& filename, EExistPolicy policy) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(filename.MountingPoint(), _nodes);
    return (readable)
        ? readable->FileExists(filename, policy)
        : false;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemTrie::FileStats(FFileStat* pstat, const FFilename& filename) const {
    Assert(pstat);
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(filename.MountingPoint(), _nodes);
    return (readable)
        ? readable->FileStats(pstat, filename)
        : false;
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemTrie::EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(dirpath.MountingPoint(), _nodes);
    return (readable)
        ? readable->EnumerateFiles(dirpath, recursive, foreach)
        : 0;
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemTrie::GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(dirpath.MountingPoint(), _nodes);
    return (readable)
        ? readable->GlobFiles(dirpath, pattern, recursive, foreach)
        : 0;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemTrie::CreateDirectory(const FDirpath& dirpath) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentWritable* const writable = WritableComponent_(dirpath.MountingPoint(), _nodes);
    return (writable)
        ? writable->CreateDirectory(dirpath)
        : false;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemTrie::MoveFile(const FFilename& src, const FFilename& dst) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentWritable* const writableSrc = WritableComponent_(src.MountingPoint(), _nodes);
    IVirtualFileSystemComponentWritable* const writableDst = WritableComponent_(dst.MountingPoint(), _nodes);
    if (writableSrc != writableDst) {
        LOG(VFS, Warning, L"can't move a file betwean different filesystem components : {0} -> {1}", src, dst);
        return false;
    }
    else {
        return writableDst->MoveFile(src, dst);
    }
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemTrie::RemoveDirectory(const FDirpath& dirpath, bool force/* = true */) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentWritable* const writable = WritableComponent_(dirpath.MountingPoint(), _nodes);
    return (writable)
        ? writable->RemoveDirectory(dirpath, force)
        : false;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemTrie::RemoveFile(const FFilename& filename) const {
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentWritable* const writable = WritableComponent_(filename.MountingPoint(), _nodes);
    return (writable)
        ? writable->RemoveFile(filename)
        : false;
}
//----------------------------------------------------------------------------
UStreamReader FVirtualFileSystemTrie::OpenReadable(const FFilename& filename, EAccessPolicy policy) const {
    Assert(!(policy ^ EAccessPolicy::Compress)); // not handled here
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadable* const readable = ReadableComponent_(filename.MountingPoint(), _nodes);
    UStreamReader result;
    if (readable)
        result = readable->OpenReadable(filename, policy);
    if (not result)
        LOG(VFS, Error, L"failed to open file '{0}' for read (access flags = {1})", filename, policy);
    return result;
}
//----------------------------------------------------------------------------
UStreamWriter FVirtualFileSystemTrie::OpenWritable(const FFilename& filename, EAccessPolicy policy) const {
    Assert(!(policy ^ EAccessPolicy::Compress)); // not handled here
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentWritable* const writable = WritableComponent_(filename.MountingPoint(), _nodes);
    UStreamWriter result;
    if (writable)
        result = writable->OpenWritable(filename, policy);
    if (not result)
        LOG(VFS, Error, L"failed to open file '{0}' for write (access flags = {1})", filename, policy);
    return result;
}
//----------------------------------------------------------------------------
UStreamReadWriter FVirtualFileSystemTrie::OpenReadWritable(const FFilename& filename, EAccessPolicy policy) const {
    Assert(!(policy ^ EAccessPolicy::Compress)); // not handled here
    READSCOPELOCK(_barrier);
    IVirtualFileSystemComponentReadWritable* const readWritable = ReadWritableComponent_(filename.MountingPoint(), _nodes);
    UStreamReadWriter result;
    if (readWritable)
        result = readWritable->OpenReadWritable(filename, policy);
    if (not result)
        LOG(VFS, Error, L"failed to open file '{0}' for read/write (access flags = {1})", filename, policy);
    return result;
}
//----------------------------------------------------------------------------
FWString FVirtualFileSystemTrie::Unalias(const FFilename& aliased) const {
    READSCOPELOCK(_barrier);
    FVirtualFileSystemComponent* const component = VFSComponent_(aliased.MountingPoint(), _nodes);
    FWString result;
    if (component)
        result = component->Unalias(aliased);
    return result;
}
//----------------------------------------------------------------------------
void FVirtualFileSystemTrie::Clear() {
    WRITESCOPELOCK(_barrier);
    for (TPair<FMountingPoint, PVirtualFileSystemComponent>& it : _nodes)
        RemoveRef_AssertReachZero(it.second);

    _nodes.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void FVirtualFileSystemTrie::Mount(FVirtualFileSystemComponent* component) {
    Assert(component);
    Assert(component->Alias().HasMountingPoint());
    WRITESCOPELOCK(_barrier);
    _nodes.Insert_AssertUnique(component->Alias().MountingPoint(), component);
}
//----------------------------------------------------------------------------
void FVirtualFileSystemTrie::Unmount(FVirtualFileSystemComponent* component) {
    Assert(component);
    Assert(component->Alias().HasMountingPoint());
    LOG(VFS, Info, L"unmount component '{0}'", component->Alias());
    WRITESCOPELOCK(_barrier);
    _nodes.Remove_AssertExists(component->Alias().MountingPoint(), component);
}
//----------------------------------------------------------------------------
FVirtualFileSystemComponent* FVirtualFileSystemTrie::MountNativePath(const FDirpath& alias, const FWStringView& nativepPath) {
    Assert(not nativepPath.empty());
    const SVirtualFileSystemComponent component = NEW_REF(FileSystem, FVirtualFileSystemNativeComponent)(alias, FWString(nativepPath));
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
FVirtualFileSystemComponent *FVirtualFileSystemTrie::MountNativePath(const FDirpath& alias, FWString&& nativepPath) {
    Assert(nativepPath.size());
    LOG(VFS, Info, L"mount native component '{0}' -> '{1}'", alias, nativepPath);
    FVirtualFileSystemComponent *component = NEW_REF(FileSystem, FVirtualFileSystemNativeComponent)(alias, std::move(nativepPath));
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
FVirtualFileSystemComponent *FVirtualFileSystemTrie::MountNativePath(const FDirpath& alias, const FWString& nativepPath) {
    Assert(nativepPath.size());
    LOG(VFS, Info, L"mount native component '{0}' -> '{1}'", alias, nativepPath);
    FVirtualFileSystemComponent *component = NEW_REF(FileSystem, FVirtualFileSystemNativeComponent)(alias, nativepPath);
    Mount(component);
    return component;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
