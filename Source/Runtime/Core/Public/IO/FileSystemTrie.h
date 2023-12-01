#pragma once

#include "Core_fwd.h"

#include "IO/FileSystem_fwd.h"
#include "IO/FileSystemToken.h"

#include "Allocator/SlabAllocator.h"
#include "Allocator/SlabHeap.h"

#include "Maths/PrimeNumbers.h"
#include "Memory/MemoryView.h"
#include "Meta/Singleton.h"

#include "Thread/ConcurrentHashMap.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFileSystemNode {
public:
    using FGenealogy = TPrimeNumberProduct<FFileSystemNode, true>;
    using FPath = TMemoryView<const FFileSystemToken>;

    FFileSystemNode(const FFileSystemNode&) = delete;
    FFileSystemNode& operator =(const FFileSystemNode&) = delete;

    PFileSystemNode Parent() const { return _parent; }

    size_t Depth() const { return _depth; }
    hash_t HashValue() const { return _hashValue; }
    FGenealogy Genealogy() const { return _genealogy; }

    bool IsMountingPoint() const { return (1 == _depth && _hasMountingPoint); }
    bool HasMountingPoint() const { return _hasMountingPoint; }

    FPath MakeView() const {
        return { reinterpret_cast<const FFileSystemToken*>(this + 1), _depth };
    }

    NODISCARD bool Greater(const FFileSystemNode& other) const NOEXCEPT {
        const FPath lhs = MakeView();
        const FPath rhs = other.MakeView();
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(),
            rhs.begin(), rhs.end(),
            Meta::TGreater<FFileSystemToken>{});
    }

    NODISCARD bool Less(const FFileSystemNode& other) const NOEXCEPT {
        const FPath lhs = MakeView();
        const FPath rhs = other.MakeView();
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(),
            rhs.begin(), rhs.end(),
            Meta::TLess<FFileSystemToken>{});
    }

    NODISCARD bool IsChildOf(const FFileSystemNode& parent) const NOEXCEPT {
        return _genealogy.Contains(parent._genealogy);
    }

private:
    friend class FFileSystemTrie;

    FFileSystemNode(
        PFileSystemNode parent,
        hash_t hashValue,
        size_t depth,
        size_t uid,
        bool hasMountingPoint) NOEXCEPT;

    const PFileSystemNode _parent;
    const hash_t _hashValue{ 0 };
    const FGenealogy _genealogy;
    const u32 _depth : 31;
    const u32 _hasMountingPoint : 1;
};
//----------------------------------------------------------------------------
class FFileSystemTrie : private Meta::TSingleton<FFileSystemTrie> {
    friend class Meta::TSingleton<FFileSystemTrie>;
    using singleton_type = Meta::TSingleton<FFileSystemTrie>;
    static PPE_CORE_API DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;
public:
    using FGenealogy = FFileSystemNode::FGenealogy;
    using FPath = FFileSystemNode::FPath;

    PPE_CORE_API ~FFileSystemTrie();

    NODISCARD PPE_CORE_API PFileSystemNode GetIFP(const FPath& path) const NOEXCEPT;

    NODISCARD PPE_CORE_API PFileSystemNode Concat(const PFileSystemNode& baseDir, const FFileSystemToken& append);
    NODISCARD PPE_CORE_API PFileSystemNode Concat(const PFileSystemNode& baseDir, const FPath& relative);
    NODISCARD PPE_CORE_API PFileSystemNode GetOrCreate(const FPath& path);

public: // Singleton
    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif
    using singleton_type::Destroy;

    static void Create() { singleton_type::Create(); }

private:
    FFileSystemTrie();

    struct path_hasher_t {
        hash_t operator ()(const FPath& value) const NOEXCEPT {
            return hash_view(value);
        }
    };
    struct path_equalto_t {
        bool operator ()(const FPath& lhs, const FPath& rhs) const NOEXCEPT {
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }
    };

    using path_hashmap_t = TConcurrentHashMap<
        FPath,
        PFileSystemNode,
        path_hasher_t,
        path_equalto_t,
        TSlabAllocator<ALLOCATOR(FileSystem)>
    >;

    SLABHEAP(FileSystem) _heap;
    path_hashmap_t _hashMap;
    size_t _nextNodeUid{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
