#include "stdafx.h"

#include "IO/FileSystemTrie.h"

#include "Allocator/Alloca.h"
#include "Container/Hash.h"
#include "Container/Stack.h"
#include "Container/Vector.h"
#include "IO/FileSystem.h"

#include <algorithm>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static size_t ExpandFileSystemNode_(
    const TMemoryView<FFileSystemToken>& tokens,
    const FFileSystemNode* pnode,
    const FFileSystemNode* proot ) {
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
FFileSystemNode::FFileSystemNode() NOEXCEPT
:   _parent(nullptr)
,   _child(nullptr)
,   _sibbling(nullptr)
,   _leaf(nullptr)
,   _depth(0)
,   _hashValue(PPE_HASH_VALUE_SEED)
,   _sortValue(-1.0)
{}
//----------------------------------------------------------------------------
FFileSystemNode::FFileSystemNode(FFileSystemNode& parent, const FFileSystemToken& token, double sortValue, size_t uid) NOEXCEPT
:   _parent(&parent)
,   _child(nullptr)
,   _sibbling(nullptr)
,   _leaf(nullptr)
,   _token(token)
,   _depth(parent._depth + 1)
,   _hashValue(hash_tuple(parent._hashValue, token))
,   _sortValue(sortValue)
,   _genealogy(FGenealogy::Combine(parent._genealogy, FGenealogy::Prime(uid))) {
    Assert_NoAssume(-1.0 <= _sortValue && _sortValue <= 1.0);
}
//----------------------------------------------------------------------------
bool FFileSystemNode::Greater(const FFileSystemNode& other) const {
    return (_sortValue > other._sortValue);
}
//----------------------------------------------------------------------------
bool FFileSystemNode::Less(const FFileSystemNode& other) const {
    return (_sortValue < other._sortValue);
}
//----------------------------------------------------------------------------
bool FFileSystemNode::IsChildOf(const FFileSystemNode& parent) const {
    return _genealogy.Contains(parent._genealogy);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFileSystemTrie::FFileSystemTrie()
:   _numNodes(0) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE(); // handled by trie at destruction
    _root._child = _root._leaf = new (_heap) FFileSystemNode{ _root, FFileSystemToken{}, 1.0, 0 };
}
//----------------------------------------------------------------------------
FFileSystemTrie::~FFileSystemTrie() {
    Clear_ReleaseMemory_();
}
//----------------------------------------------------------------------------
FFileSystemNode* FFileSystemTrie::CreateNode_(
    FFileSystemNode& parent,
    const FFileSystemToken& token,
    const FFileSystemNode& prev,
    const FFileSystemNode& next) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE(); // handled by trie at destruction
    const double s = ((prev._sortValue + next._sortValue) * 0.5);
    Assert_NoAssume(s > prev._sortValue);
    Assert_NoAssume(s < next._sortValue);
    return new (_heap) FFileSystemNode{
        parent, token, s, token.empty() ? 0 : _numNodes++ };
}
//----------------------------------------------------------------------------
const FFileSystemNode* FFileSystemTrie::Get_(const FFileSystemNode& root, const FFileSystemToken& token) const {
    Assert_NoAssume(not token.empty());

    for (const FFileSystemNode* n = root._child; n != root._leaf; n = n->_sibbling) {
        if (n->_token == token)
            return n;
        else if (token < n->_token)
            break;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
FFileSystemNode* FFileSystemTrie::GetOrCreate_(FFileSystemNode& root, const FFileSystemToken& token) {
    Assert_NoAssume(not token.empty());

    FFileSystemNode* n = root._child;
    if (nullptr == n) {
        auto* const l = CreateNode_(root, FFileSystemToken{}, root, *root._sibbling);
        auto* const r = CreateNode_(root, token, root, *l);

        root._child = r;
        root._leaf = l;
        r->_sibbling = l;

        return r;
    }
    else if (n->_token.empty() || token < n->_token) {
        auto* const r = CreateNode_(root, token, root, *n);

        r->_sibbling = n;
        n->_parent->_child = r;

        return r;
    }
    else {
        FFileSystemNode* p = n;
        n = n->_sibbling;

        while (n != root._leaf && n->_token < token) {
            p = n;
            n = n->_sibbling;
        }

        if (n->_token == token) return n;
        if (p->_token == token) return p;

        auto* const r = CreateNode_(root, token, p->_leaf ? *p->_leaf : *p, *n);
        p->_sibbling = r;
        r->_sibbling = n;

        return r;
    }
}
//----------------------------------------------------------------------------
const FFileSystemNode *FFileSystemTrie::GetIFP(const TMemoryView<const FFileSystemToken>& path) const {
    Assert_NoAssume(not path.empty());

    const FFileSystemNode* n = &_root;

    READSCOPELOCK(_barrier);

    for (const FFileSystemToken& token : path) {
        n = Get_(*n, token);
        if (nullptr == n)
            return nullptr;
    }

    Assert(n);
    return n;
}
//----------------------------------------------------------------------------
const FFileSystemNode *FFileSystemTrie::Concat(const FFileSystemNode* basedir, const FFileSystemToken& append) {
    return Concat(basedir, MakeView(&append, &append + 1));
}
//----------------------------------------------------------------------------
const FFileSystemNode *FFileSystemTrie::Concat(const FFileSystemNode* basedir, const TMemoryView<const FFileSystemToken>& path) {
    if (path.empty())
        return basedir;

    FFileSystemNode* n = (basedir ? const_cast<FFileSystemNode*>(basedir) : &_root);

    WRITESCOPELOCK(_barrier);

    for (const FFileSystemToken& token : path)
        n = GetOrCreate_(*n, token);

    return n;
}
//----------------------------------------------------------------------------
void FFileSystemTrie::Clear_ReleaseMemory_() {
    Assert_NoAssume(_root._child);
    Assert_NoAssume(not _root._sibbling);

    WRITESCOPELOCK(_barrier);

#if 0
    ONLY_IF_ASSERT(size_t numDeleteds = 0);
    STACKLOCAL_POD_STACK(FFileSystemNode*, queue, 128);

    queue.Push(_root._child);

    FFileSystemNode* n;
    for (;;) {
        if (not queue.Pop(&n))
            break;

        Assert(n);

        if (n->_child)
            queue.Push(n->_child);
        if (n->_sibbling)
            queue.Push(n->_sibbling);

        Meta::Destroy(n);
        allocator_traits::DeallocateOneT(*this, n);

        ONLY_IF_ASSERT(++numDeleteds);
    }

    Assert_NoAssume(numDeleteds >= _numNodes);

#else
    // nodes are trivially destructible, release all in one call
    _heap.ReleaseAll();

#endif
}
//----------------------------------------------------------------------------
void FFileSystemTrie::Clear() {
    Clear_ReleaseMemory_();

    PPE_LEAKDETECTOR_WHITELIST_SCOPE(); // handled by trie at destruction

    _numNodes = 0;
    _root._child = _root._leaf = new (_heap) FFileSystemNode{ _root, FFileSystemToken{}, 1.0, 0 };
}
//----------------------------------------------------------------------------
const FFileSystemNode& FFileSystemTrie::FirstNode(const FFileSystemNode& pnode) const {
    Assert_NoAssume(&_root != &pnode);

    READSCOPELOCK(_barrier);

    const FFileSystemNode* n = &pnode;
    for (; n->_parent != &_root; n = n->_parent);

    Assert(n);
    Assert_NoAssume(n->_parent == &_root);
    return (*n);
}
//----------------------------------------------------------------------------
size_t FFileSystemTrie::Expand(const TMemoryView<FFileSystemToken>& tokens, const FFileSystemNode *pnode) const {
    Assert_NoAssume(not tokens.empty());

    if (nullptr == pnode)
        return 0;

    READSCOPELOCK(_barrier);

    return ExpandFileSystemNode_(tokens, pnode, &_root);
}
//----------------------------------------------------------------------------
size_t FFileSystemTrie::Expand(const TMemoryView<FFileSystemToken>& tokens, const FFileSystemNode *pbegin, const FFileSystemNode *pend) const {
    Assert_NoAssume(not tokens.empty());
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
} //!namespace PPE
