#pragma once

#include "Core/Container/Trie.h"

#include "Core/Container/RingBuffer.h"
#include "Core/Container/Stack.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
TrieNode<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::TrieNode() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
TrieNode<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::~TrieNode() {
    Assert(_children.empty());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
bool TrieNode<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::Find(const _Key& key, TrieNode **node) {
    return _children.Find(key, reinterpret_cast<void **>(node));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
bool TrieNode<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::Find(const _Key& key, const TrieNode **node) const {
    return _children.Find(key, (void **)(node));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
Trie<_Key, _Value,  _InSituCount, _EqualTo, _Allocator>::Trie()
:   _root(nullptr), _size(0) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
Trie<_Key, _Value,  _InSituCount, _EqualTo, _Allocator>::~Trie() {
    Clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
auto Trie<_Key, _Value,  _InSituCount, _EqualTo, _Allocator>::Insert_AssertUnique(const MemoryView<const _Key>& keys, _Value&& rvalue) -> const node_type * {
    node_type *const node = CreatePathIFN_(keys);
    Assert(_Value() == node->_value);
    node->_value = std::move(rvalue);
    ++_size;
    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
auto Trie<_Key, _Value,  _InSituCount, _EqualTo, _Allocator>::Insert_AssertUnique(const MemoryView<const _Key>& keys, const _Value& value) -> const node_type * {
    node_type *const node = CreatePathIFN_(keys);
    Assert(_Value() == node->_value);
    node->_value = value;
    ++_size;
    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
auto Trie<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::Insert_OverwriteIFN(const MemoryView<const _Key>& keys, _Value&& rvalue) -> const node_type *{
    node_type *const node = CreatePathIFN_(keys);
    node->_value = std::move(rvalue);
    ++_size;
    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
auto Trie<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::Insert_OverwriteIFN(const MemoryView<const _Key>& keys, const _Value& value) -> const node_type *{
    node_type *const node = CreatePathIFN_(keys);
    node->_value = value;
    ++_size;
    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
size_t Trie<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::Follow_ReturnDepth(const node_type **pLastNode, const MemoryView<const _Key>& keys) const {
    Assert(pLastNode);

    if (keys.empty() || nullptr == _root) {
        *pLastNode = nullptr;
        return 0;
    }

    size_t depth = 0;
    node_type *node = _root;
    for (const _Key& key : keys) {
        node_type *child = nullptr;
        if (!node->Find(key, &child))
            return depth;

        Assert(child);
        node = child;
        ++depth;
    }

    *pLastNode = node;
    return depth;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
void Trie<_Key, _Value,  _InSituCount, _EqualTo, _Allocator>::Clear() {
    if (nullptr == _root) {
        Assert(0 == _size);
        return;
    }

    STACKLOCAL_POD_STACK(node_type *, nodes, 32);

    node_type *node = _root;
    do {
        Assert(node);
        for (const Pair<_Key, void *>& it : node->_children)
            nodes.Push(reinterpret_cast<node_type *>(it.second));

        allocator_type::destroy(node);
        allocator_type::deallocate(node, 1);
    } while (nodes.Pop(&node));

    _size = 0;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
auto Trie<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::CreatePathIFN_(const MemoryView<const _Key>& keys) -> node_type * {
    Assert(keys.size());

    if (nullptr == _root) {
        _root = allocator_type::allocate(1);
        allocator_type::construct(_root);
    }
    Assert(_root);

    node_type *node = _root;
    for (const _Key& key : keys) {
        node_type *child = nullptr;
        if (!node->Find(key, &child)) {
            child = allocator_type::allocate(1);
            allocator_type::construct(child);
            node->_children.Get(key) = child;
        }
        Assert(child);
        node = child;
    }

    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
template <typename _Functor>
size_t Trie<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::EachNode_BreadthFirst(_Functor&& functor) const {
    if (nullptr == _root)
        return 0;

    size_t count = 0;

    STACKLOCAL_POD_STACK(const node_type *, stack, 32);
    stack.Push(_root);

    node_type *node = nullptr;
    while (stack.Pop(&node)) {
        Assert(node);
        if (functor(node))
            continue;

        ++count;
        reverseforeachitem(it, node->_children)
            stack.Push(it->second);
    }

    return count;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
template <typename _Functor>
size_t Trie<_Key, _Value, _InSituCount, _EqualTo, _Allocator>::EachNode_DepthFirst(_Functor&& functor) const {
    if (nullptr == _root)
        return 0;

    size_t count = 0;

    STACKLOCAL_POD_RINGBUFFER(const node_type *, queue, 64);
    queue.Queue(_root);

    node_type *node = nullptr;
    while (queue.Dequeue(&node)) {
        Assert(node);
        if (!functor(node))
            continue;

        ++count;
        foreachitem(it, node->_children)
            queue.Queue(it->second);
    }

    return count;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
