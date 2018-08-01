#include "stdafx.h"

#include "FileSystemTrie.h"

#include "Allocator/Alloca.h"
#include "Allocator/PoolAllocator-impl.h"
#include "Container/Hash.h"
#include "Container/Stack.h"
#include "Container/Vector.h"
#include "IO/FileSystem.h"

#include <algorithm>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static size_t ExpandFileSystemNode_(
    const TMemoryView<FFileSystemToken>& tokens,
    const FFileSystemNode *pnode,
    const FFileSystemNode *proot ) {
    if (nullptr == pnode)
        return 0;

    size_t count = 0;
    for (; proot != pnode; pnode = pnode->Parent())
        tokens[count++] = pnode->Token();

    Assert(tokens.size() == count);
    std::reverse(tokens.begin(), tokens.begin() + count);

    return count;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(FileSystem, FFileSystemNode, );
//----------------------------------------------------------------------------
FFileSystemNode::FFileSystemNode(const FFileSystemNode *parent, const FFileSystemToken& token)
:   _parent(parent)
,   _token(token) {
    if (_parent) {
        _depth = _parent->_depth + 1;
        _hashValue = hash_tuple(_parent->Token().HashValue(), _token.HashValue() );
    }
    else {
        _depth = 0;
        _hashValue = _token.HashValue();
    }
}
//----------------------------------------------------------------------------
FFileSystemNode::~FFileSystemNode() {}
//----------------------------------------------------------------------------
bool FFileSystemNode::IsChildOf(const FFileSystemNode *parent) const {
    Assert(parent);

    const FFileSystemNode *n = _parent.get();
    for (; n && n != parent; n = n->_parent.get());

    return (n == parent);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFileSystemTrie::FFileSystemTrie()
:   _root(new FFileSystemNode(nullptr, FFileSystemToken()))
{}
//----------------------------------------------------------------------------
FFileSystemTrie::~FFileSystemTrie() {
    Clear();
    RemoveRef_AssertReachZero(_root);
}
//----------------------------------------------------------------------------
const FFileSystemNode *FFileSystemTrie::GetIFP(const TMemoryView<const FFileSystemToken>& path) const {
    Assert(!path.empty());

    typedef TMemoryView<const FFileSystemToken>::iterator iterator;

    iterator bpath = path.begin();
    const iterator epath = path.end();

    READSCOPELOCK(_barrier);

    const FFileSystemNode *node = _root->_child.get();
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
const FFileSystemNode *FFileSystemTrie::Concat(const FFileSystemNode *basedir, const FFileSystemToken& append) {
    return Concat(basedir, MakeView(&append, &append + 1));
}
//----------------------------------------------------------------------------
const FFileSystemNode *FFileSystemTrie::Concat(const FFileSystemNode *basedir, const TMemoryView<const FFileSystemToken>& path) {
    if (path.empty())
        return basedir;

    Assert(!path.empty());

    typedef TMemoryView<const FFileSystemToken>::iterator iterator;

    iterator bpath = path.begin();
    const iterator epath = path.end();
    Assert(!bpath->empty());

    WRITESCOPELOCK(_barrier);

    FFileSystemNode *node = nullptr;
    FFileSystemNode *parent = nullptr;

    if (nullptr == basedir) {
        Assert(_root);
        node = _root->_child.get();
        parent = _root.get();
    }
    else {
        node = remove_const(basedir->_child.get());
        parent = remove_const(basedir);
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
            node = new FFileSystemNode(parent, *bpath);
            node->_sibbling = parent->_child;
            parent->_child = node;
            parent = node;
        }
    }

    return node;
}
//----------------------------------------------------------------------------
void FFileSystemTrie::Clear() {
    Assert(_root);

    WRITESCOPELOCK(_barrier);

    Assert(!_root->_sibbling);

    if (!_root->_child)
        return;

    // we want to check ref count to prevent deleting nodes still referenced

    STACKLOCAL_POD_STACK(FFileSystemNode *, queue, 128); {
        FFileSystemNode *const next = _root->_child.get();
        AddRef(next);
        _root->_child.reset();
        queue.Push(next);
    }

    VECTOR(FileSystem, PFileSystemNode) nodes;
    nodes.reserve(128);

    // first-pass : detach all nodes from each others
    for (;;) {
        FFileSystemNode *node = nullptr;
        if (!queue.Pop(&node))
            break;
        Assert(node);
        Assert(node->_parent);

        if (node->_sibbling) {
            Assert(node->_sibbling->_parent == node->_parent);
            FFileSystemNode *const next = node->_sibbling.get();
            AddRef(next);
            node->_sibbling.reset();
            queue.Push(next);
        }

        if (node->_child) {
            Assert(node->_child->_parent == node);
            FFileSystemNode *const next = node->_child.get();
            AddRef(next);
            node->_child.reset();
            queue.Push(next);
        }

        node->_parent.reset();
        nodes.push_back(node);

        RemoveRef(node);
    }

    // second-pass : decrement ref count of each node and assert destruction
    for (PFileSystemNode& node : nodes)
        RemoveRef_AssertReachZero(node);

    nodes.clear();
}
//----------------------------------------------------------------------------
const FFileSystemNode* FFileSystemTrie::RootNode(const FFileSystemNode *pnode) const {
    if (nullptr == pnode)
        return nullptr;

    READSCOPELOCK(_barrier);

    for (   const FFileSystemNode* pparent = pnode->Parent();
            pparent != _root;
            pnode = pparent, pparent = pnode->Parent() ) {
        Assert(pparent);
    }

    Assert(pnode);
    Assert(pnode->Parent() == _root);
    return pnode;
}
//----------------------------------------------------------------------------
size_t FFileSystemTrie::Expand(const TMemoryView<FFileSystemToken>& tokens, const FFileSystemNode *pnode) const {
    Assert(tokens.size() > 0);

    if (nullptr == pnode)
        return 0;

    READSCOPELOCK(_barrier);

    return ExpandFileSystemNode_(tokens, pnode, _root.get() );
}
//----------------------------------------------------------------------------
size_t FFileSystemTrie::Expand(const TMemoryView<FFileSystemToken>& tokens, const FFileSystemNode *pbegin, const FFileSystemNode *pend) const {
    Assert(tokens.size() > 0);
    Assert(pbegin);
    Assert(pend);

    if (pbegin == pend)
        return 0;

    READSCOPELOCK(_barrier);

    return ExpandFileSystemNode_(tokens, pend, pbegin);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
