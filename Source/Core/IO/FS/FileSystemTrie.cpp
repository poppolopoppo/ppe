#include "stdafx.h"

#include "FileSystemTrie.h"

#include "Allocator/Alloca.h"
#include "Allocator/PoolAllocator-impl.h"
#include "Container/Hash.h"
#include "Container/Vector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static size_t ExpandFileSystemNode_(FileSystemToken *ptokens, size_t capacity, const FileSystemNode *node, const FileSystemNode *root) {
    Assert(node);
    Assert(!node->Token().empty());

    size_t depth = 0;
    Assert(node->Parent());
    if (node->Parent() != root)
        depth = ExpandFileSystemNode_(ptokens, capacity, node->Parent(), root);

    Assert(depth < capacity);
    ptokens[depth] = node->Token();

    return depth + 1;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(FileSystemNode, );
//----------------------------------------------------------------------------
FileSystemNode::FileSystemNode(const FileSystemNode *parent, const FileSystemToken& token)
:   _parent(parent)
,   _token(token) {
    _hashValue = (_parent)
        ? hash_value(_parent->Token().HashValue(), _token.HashValue())
        : _token.HashValue();
}
//----------------------------------------------------------------------------
FileSystemNode::~FileSystemNode() {}
//----------------------------------------------------------------------------
bool FileSystemNode::IsChildOf(const FileSystemNode *parent) const {
    Assert(parent);

    const FileSystemNode *n = _parent.get();
    for (; n && n != parent; n = n->_parent.get());

    return (n == parent);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FileSystemTrie::FileSystemTrie()

:   _root(new FileSystemNode(nullptr, FileSystemToken()))
{}
//----------------------------------------------------------------------------
FileSystemTrie::~FileSystemTrie() {
    Clear();
    RemoveRef_AssertReachZero(_root);
}
//----------------------------------------------------------------------------
const FileSystemNode *FileSystemTrie::GetIFP(const MemoryView<const FileSystemToken>& path) const {
    Assert(!path.empty());

    const FileSystemToken *bpath = path.begin();
    const FileSystemToken *epath = path.end();

    std::lock_guard<std::mutex> scopeLock(_barrier);

    const FileSystemNode *node = _root->_child.get();
    while (node) {
        Assert(!bpath->empty());
        for (; node; node = node->_sibbling.get())
            if (node->Token() == *bpath) {
                if (++bpath == epath)
                    return node;

                node = node->_child.get();
                break;
            }
    }

    return nullptr;
}
//----------------------------------------------------------------------------
const FileSystemNode *FileSystemTrie::Concat(const FileSystemNode *basedir, const FileSystemToken& append) {
    return Concat(basedir, MakeView(&append, &append + 1));
}
//----------------------------------------------------------------------------
const FileSystemNode *FileSystemTrie::Concat(const FileSystemNode *basedir, const MemoryView<const FileSystemToken>& path) {
    if (path.empty())
        return basedir;

    Assert(!path.empty());

    const FileSystemToken *bpath = path.begin();
    const FileSystemToken *epath = path.end();
    Assert(!bpath->empty());

    std::lock_guard<std::mutex> scopeLock(_barrier);

    FileSystemNode *node = nullptr;
    FileSystemNode *parent = nullptr;

    if (nullptr == basedir) {
        Assert(_root);
        node = _root->_child.get();
        parent = _root.get();
    }
    else {
        node = const_cast<FileSystemNode *>(basedir->_child.get());
        parent = const_cast<FileSystemNode *>(basedir);
    }
    Assert(parent);

    while (node)
        if (node->Token() == *bpath) {
            if (++bpath == epath)
                break;

            Assert(!bpath->empty());
            parent = node;
            node = node->_child.get();
        }
        else {
            node = node->_sibbling.get();
        }

    if (bpath == epath) {
        Assert(node);
        Assert(node->Token() == path.back());
    }
    else {
        Assert(bpath < epath);
        Assert(parent);
        Assert(nullptr == node);

        for (; bpath != epath; ++bpath) {
            Assert(!bpath->empty());
            node = new FileSystemNode(parent, *bpath);
            node->_sibbling = parent->_child;
            parent->_child = node;
            parent = node;
        }
    }

    return node;
}
//----------------------------------------------------------------------------
void FileSystemTrie::Clear() {
    Assert(_root);

    std::lock_guard<std::mutex> scopeLock(_barrier);

    Assert(!_root->_sibbling);

    if (!_root->_child)
        return;

    // we want to check ref count to prevent deleting nodes still referenced

    MALLOCA_STACK(FileSystemNode *, queue, 128); {
        FileSystemNode *const next = _root->_child.get();
        AddRef(next);
        _root->_child.reset();
        queue.PushPOD(next);
    }

    VECTOR_THREAD_LOCAL(FileSystem, PFileSystemNode) nodes;
    nodes.reserve(128);

    // first-pass : detach all nodes from each others
    do {
        FileSystemNode *node = nullptr;
        if (!queue.PopPOD(&node))
            break;
        Assert(node);
        Assert(node->_parent);

        if (node->_sibbling) {
            Assert(node->_sibbling->_parent == node->_parent);
            FileSystemNode *const next = node->_sibbling.get();
            AddRef(next);
            node->_sibbling.reset();
            queue.PushPOD(next);
        }

        if (node->_child) {
            Assert(node->_child->_parent == node);
            FileSystemNode *const next = node->_child.get();
            AddRef(next);
            node->_child.reset();
            queue.PushPOD(next);
        }

        node->_parent.reset();
        nodes.push_back(node);

        RemoveRef(node);
    }
    while (true);

    // second-pass : decrement ref count of each node and assert destruction
    for (PFileSystemNode& node : nodes)
        RemoveRef_AssertReachZero(node);

    nodes.clear();
}
//----------------------------------------------------------------------------
size_t FileSystemTrie::Expand(FileSystemToken *ptokens, size_t capacity, const FileSystemNode *pnode) const {
    Assert(ptokens);
    Assert(capacity > 0);

    if (nullptr == pnode)
        return 0;

    // should be ok without lock ... (nodes are not muted after their creation)
    //std::lock_guard<std::mutex> scopeLock(_barrier);

    return ExpandFileSystemNode_(ptokens, capacity, pnode, _root);
}
//----------------------------------------------------------------------------
size_t FileSystemTrie::Expand(FileSystemToken *ptokens, size_t capacity, const FileSystemNode *pbegin, const FileSystemNode *pend) const {
    Assert(ptokens);
    Assert(capacity > 0);
    Assert(pbegin);
    Assert(pend);

    if (pbegin == pend)
        return 0;

    // should be ok without lock ... (nodes are not muted after their creation)
    //std::lock_guard<std::mutex> scopeLock(_barrier);

    return ExpandFileSystemNode_(ptokens, capacity, pend, pbegin);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
