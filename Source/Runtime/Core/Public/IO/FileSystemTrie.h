#pragma once

#include "Core.h"

#include "IO/FileSystemToken.h"

#include "Allocator/LinearHeap.h"
#include "Memory/MemoryView.h"
#include "Memory/RefPtr.h"
#include "Maths/PrimeNumbers.h"
#include "Meta/Singleton.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FFileSystemNode {
public:
    using FGenealogy = TPrimeNumberProduct<FFileSystemNode, true>;

    FFileSystemNode() NOEXCEPT;
    FFileSystemNode(
        FFileSystemNode& parent,
        const FFileSystemToken& token,
        double sortValue, size_t uid,
        bool isMountingPoint ) NOEXCEPT;

    FFileSystemNode(const FFileSystemNode& ) = delete;
    FFileSystemNode& operator =(const FFileSystemNode& ) = delete;

    const FFileSystemNode* Parent() const { return _parent; }
    const FFileSystemNode* Child() const { return _child; }
    const FFileSystemNode* Sibbling() const { return _sibbling; }
    const FFileSystemNode* Leaf() const { return _leaf; }

    bool IsMountingPoint() const { return (1 == _depth && HasMountingPoint()); }
    bool HasMountingPoint() const { return (_flags & EInternalFlags::HasMountingPoint); }

    bool IsTail() const { return _token.empty(); }
    const FFileSystemToken& Token() const { return _token; }

    size_t Depth() const { return _depth; }
    hash_t HashValue() const { return _hashValue; }
    double SortValue() const { return _sortValue; }
    FGenealogy Genealogy() const { return _genealogy; }

    bool Greater(const FFileSystemNode& other) const;
    bool Less(const FFileSystemNode& other) const;
    bool IsChildOf(const FFileSystemNode& parent) const;

private:
    friend class FFileSystemTrie;

    enum class EInternalFlags : u32 {
        None = 0,
        HasMountingPoint = 1<<0,
    };
    ENUM_FLAGS_FRIEND(EInternalFlags);

    FFileSystemNode* _parent;
    FFileSystemNode* _child;
    FFileSystemNode* _sibbling;
    FFileSystemNode* _leaf;

    const FFileSystemToken _token;
    const u32 _depth;
    const EInternalFlags _flags;
    const hash_t _hashValue;
    const double _sortValue;
    const FGenealogy _genealogy;
};
//----------------------------------------------------------------------------
class PPE_CORE_API FFileSystemTrie : private Meta::TSingleton<FFileSystemTrie> {
public:
    using FGenealogy = FFileSystemNode::FGenealogy;

    ~FFileSystemTrie();

    const FFileSystemNode* GetIFP(const TMemoryView<const FFileSystemToken>& path) const;
    const FFileSystemNode* Concat(const FFileSystemNode* basedir, const FFileSystemToken& append);
    const FFileSystemNode* Concat(const FFileSystemNode* basedir, const TMemoryView<const FFileSystemToken>& path);
    const FFileSystemNode* GetOrCreate(const TMemoryView<const FFileSystemToken>& path) { return Concat(nullptr, path); }

    const FFileSystemNode& FirstNode(const FFileSystemNode& pnode) const;

    size_t Expand(const TMemoryView<FFileSystemToken>& tokens, const FFileSystemNode* pnode) const; // returns actual tokens count
    size_t Expand(const TMemoryView<FFileSystemToken>& tokens, const FFileSystemNode* pbegin, const FFileSystemNode *pend) const; // returns actual tokens count

    void Clear();

public: // Singleton
    using  parent_type = Meta::TSingleton<FFileSystemTrie>;

    using parent_type::Get;
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }

private:
    friend class Meta::TSingleton<FFileSystemTrie>;

    FReadWriteLock _barrier;
    size_t _numNodes;
    FFileSystemNode _root;
    LINEARHEAP(FileSystem) _heap;

    FFileSystemTrie();

    void CreateRootNode_();
    FFileSystemNode* CreateNode_(
        FFileSystemNode& parent,
        const FFileSystemToken& token,
        const FFileSystemNode& prev,
        const FFileSystemNode& next );

    void Clear_ReleaseMemory_();
    const FFileSystemNode* Get_(const FFileSystemNode& root, const FFileSystemToken& token) const;
    FFileSystemNode* GetOrCreate_(FFileSystemNode& root, const FFileSystemToken& token);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
