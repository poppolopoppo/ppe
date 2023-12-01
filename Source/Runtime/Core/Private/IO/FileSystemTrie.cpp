// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/FileSystemTrie.h"

#include "Allocator/Alloca.h"
#include "Container/Hash.h"
#include "IO/FileSystem.h"

#include <algorithm>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool IsMountingPoint_(const FFileSystemToken& token) {
    return (not token.empty() && token.MakeView().back() == L':');
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFileSystemNode::FFileSystemNode(
    PFileSystemNode parent,
    hash_t hashValue,
    size_t depth,
    size_t uid,
    bool hasMountingPoint) NOEXCEPT
:   _parent(parent)
,   _hashValue(hashValue)
,   _genealogy(parent
        ? FGenealogy::Combine(parent->_genealogy, FGenealogy::Prime(uid))
        : FGenealogy::Prime(uid))
,   _depth(checked_cast<u32>(depth))
,   _hasMountingPoint(hasMountingPoint)
{
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FFileSystemTrie::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
FFileSystemTrie::FFileSystemTrie()
:   _hashMap(
        _heap.AllocateT<path_hashmap_t::FEntry*>(4049),
        TSlabAllocator<ALLOCATOR(FileSystem)>{ _heap })
{
}
//----------------------------------------------------------------------------
FFileSystemTrie::~FFileSystemTrie() {
    // nodes are trivially destructible, release all in one call
    _hashMap.Clear_ForgetMemory();
    _heap.ReleaseAll();
}
//----------------------------------------------------------------------------
PFileSystemNode FFileSystemTrie::GetIFP(const FPath& path) const NOEXCEPT {
    Assert_NoAssume(not path.empty());

    const auto* it = _hashMap.Lookup(path);
    if (Likely(it != nullptr))
        return it->second;

    return Default;
}
//----------------------------------------------------------------------------
PFileSystemNode FFileSystemTrie::Concat(const PFileSystemNode& baseDir, const FFileSystemToken& append) {
    return Concat(baseDir, MakeView(&append, &append + 1));
}
//----------------------------------------------------------------------------
PFileSystemNode FFileSystemTrie::Concat(const PFileSystemNode& baseDir, const FPath& relative) {
    if (relative.empty())
        return baseDir;

    if (not baseDir)
        return GetOrCreate(relative);

    STACKLOCAL_POD_ARRAY(FFileSystemToken, absolute, baseDir->_depth + relative.size());
    baseDir->MakeView().CopyTo(absolute.CutBefore(baseDir->_depth));
    relative.CopyTo(absolute.CutStartingAt(baseDir->_depth));

    return GetOrCreate(absolute);
}
//----------------------------------------------------------------------------
PFileSystemNode FFileSystemTrie::GetOrCreate(const FPath& path) {
    Assert(not path.empty());

    PFileSystemNode parent;
    if (path.size() > 1)
        parent = GetOrCreate(path.ShiftBack());

    const auto& [_, node] =
        _hashMap.FindOrAdd(path, [&](path_hashmap_t::FEntry* entryAdded) {
            entryAdded->Value.second = INPLACE_NEW(_heap.Allocate(sizeof(FFileSystemNode) + path.SizeInBytes()), FFileSystemNode)(
                parent,
                path_hasher_t{}(path),
                path.size(),
                _nextNodeUid++,
                IsMountingPoint_(path[0]));
            entryAdded->Value.first = entryAdded->Value.second->MakeView();
            FPlatformMemory::Memcpy((void*)entryAdded->Value.first.data(), path.data(), path.SizeInBytes());
        });

    return node;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
