#pragma once

#include "Core.h"

#include "IO/FS/FileSystemToken.h"

#include "Allocator/PoolAllocator.h"
#include "Container/AssociativeVector.h"
#include "Memory/MemoryView.h"
#include "Memory/RefPtr.h"
#include "Meta/Singleton.h"
#include "Thread/ReadWriteLock.h"

#include <mutex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(FileSystemNode);
class PPE_API FFileSystemNode : public FRefCountable {
    friend class FFileSystemTrie;
public:
    FFileSystemNode(const FFileSystemNode *parent, const FFileSystemToken& token);
    ~FFileSystemNode();

    FFileSystemNode(const FFileSystemNode& ) = delete;
    FFileSystemNode& operator =(const FFileSystemNode& ) = delete;

    const FFileSystemNode *Parent() const { return _parent.get(); }
    const FFileSystemNode *Child() const { return _child.get(); }
    const FFileSystemNode *Sibbling() const { return _sibbling.get(); }

    const FFileSystemToken& Token() const { return _token; }

    size_t Depth() const { return _depth; }
    size_t HashValue() const { return _hashValue; }

    bool IsChildOf(const FFileSystemNode *parent) const;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    SCFileSystemNode _parent;
    PFileSystemNode _child;
    PFileSystemNode _sibbling;
    FFileSystemToken _token;
    size_t _depth;
    size_t _hashValue;
};
//----------------------------------------------------------------------------
class PPE_API FFileSystemTrie {
public:
    FFileSystemTrie();
    ~FFileSystemTrie();

    // _root is const, no need to lock to be thread safe
    const FFileSystemNode *Root() const { return _root.get(); }

    const FFileSystemNode *GetIFP(const TMemoryView<const FFileSystemToken>& path) const;
    const FFileSystemNode *Concat(const FFileSystemNode *basedir, const FFileSystemToken& append);
    const FFileSystemNode *Concat(const FFileSystemNode *basedir, const TMemoryView<const FFileSystemToken>& path);
    const FFileSystemNode *GetOrCreate(const TMemoryView<const FFileSystemToken>& path) { return Concat(nullptr, path); }

    const FFileSystemNode* RootNode(const FFileSystemNode *pnode) const;

    size_t Expand(const TMemoryView<FFileSystemToken>& tokens, const FFileSystemNode *pnode) const; // returns actual tokens count
    size_t Expand(const TMemoryView<FFileSystemToken>& tokens, const FFileSystemNode *pbegin, const FFileSystemNode *pend) const; // returns actual tokens count

    void Clear();

private:
    FReadWriteLock _barrier;
    PFileSystemNode _root;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFileSystemPath : Meta::TSingleton<FFileSystemTrie, FFileSystemPath> {
public:
    typedef Meta::TSingleton<FFileSystemTrie, FFileSystemPath> parent_type;

    using parent_type::Get;
#ifdef WITH_PPE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
