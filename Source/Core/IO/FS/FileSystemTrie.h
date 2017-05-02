#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Singleton.h"
#include "Core/Thread/ReadWriteLock.h"

#include <mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(FileSystemNode);
class FFileSystemNode : public FRefCountable {
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
class FFileSystemTrie {
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

    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
