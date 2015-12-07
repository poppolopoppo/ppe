#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Singleton.h"

#include <mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(FileSystemNode);
class FileSystemNode : public RefCountable {
    friend class FileSystemTrie;
public:
    FileSystemNode(const FileSystemNode *parent, const FileSystemToken& token);
    ~FileSystemNode();

    FileSystemNode(const FileSystemNode& ) = delete;
    FileSystemNode& operator =(const FileSystemNode& ) = delete;

    const FileSystemNode *Parent() const { return _parent.get(); }
    const FileSystemNode *Child() const { return _child.get(); }
    const FileSystemNode *Sibbling() const { return _sibbling.get(); }

    const FileSystemToken& Token() const { return _token; }

    size_t Depth() const { return _depth; }

    size_t HashValue() const { return _hashValue; }

    bool IsChildOf(const FileSystemNode *parent) const;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    SCFileSystemNode _parent;
    PFileSystemNode _child;
    PFileSystemNode _sibbling;
    FileSystemToken _token;
    size_t _depth;
    size_t _hashValue;
};
//----------------------------------------------------------------------------
class FileSystemTrie {
public:
    FileSystemTrie();
    ~FileSystemTrie();

    // _root is const, no need to lock to be thread safe
    const FileSystemNode *Root() const { return _root.get(); }

    const FileSystemNode *GetIFP(const MemoryView<const FileSystemToken>& path) const;
    const FileSystemNode *Concat(const FileSystemNode *basedir, const FileSystemToken& append);
    const FileSystemNode *Concat(const FileSystemNode *basedir, const MemoryView<const FileSystemToken>& path);
    const FileSystemNode *GetOrCreate(const MemoryView<const FileSystemToken>& path) { return Concat(nullptr, path); }

    const FileSystemNode* RootNode(const FileSystemNode *pnode) const;
    size_t Expand(FileSystemToken *ptokens, size_t capacity, const FileSystemNode *pnode) const; // returns actual tokens count
    size_t Expand(FileSystemToken *ptokens, size_t capacity, const FileSystemNode *pbegin, const FileSystemNode *pend) const; // returns actual tokens count

    void Clear();

private:
    mutable std::mutex _barrier;
    PFileSystemNode _root;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FileSystemPath : Meta::Singleton<FileSystemTrie, FileSystemPath> {
public:
    typedef Meta::Singleton<FileSystemTrie, FileSystemPath> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
