#pragma once

#include "Core/Container/Trie.h"

#include "Core/Memory/MemoryStack.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
TrieNode<_Key, _Value, _EqualTo, _Allocator>::TrieNode() {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
TrieNode<_Key, _Value, _EqualTo, _Allocator>::~TrieNode() {
    Assert(_children.empty());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
bool TrieNode<_Key, _Value, _EqualTo, _Allocator>::Find(const _Key& key, TrieNode **node) {
    return _children.Find(key, reinterpret_cast<void **>(node));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
bool TrieNode<_Key, _Value, _EqualTo, _Allocator>::Find(const _Key& key, const TrieNode **node) const {
    return _children.Find(key, (void **)(node));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
Trie<_Key, _Value, _EqualTo, _Allocator>::Trie()
:   _root(nullptr), _size(0) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
Trie<_Key, _Value, _EqualTo, _Allocator>::~Trie() {
    Clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
auto Trie<_Key, _Value, _EqualTo, _Allocator>::Insert_AssertUnique(const MemoryView<const _Key>& keys, _Value&& rvalue) -> const node_type * {
    node_type *const node = GetMatch_(keys);
    Assert(_Value() == node->_value);
    node->_value = std::move(rvalue);
    ++_size;
    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
auto Trie<_Key, _Value, _EqualTo, _Allocator>::Insert_AssertUnique(const MemoryView<const _Key>& keys, const _Value& value) -> const node_type * {
    node_type *const node = GetMatch_(keys);
    Assert(_Value() == node->_value);
    node->_value = value;
    ++_size;
    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
auto Trie<_Key, _Value, _EqualTo, _Allocator>::GetMatch_(const MemoryView<const _Key>& keys) -> node_type * {
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
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
void Trie<_Key, _Value, _EqualTo, _Allocator>::Clear() {
    if (nullptr == _root) {
        Assert(0 == _size);
        return;
    }

    MALLOCA_STACK(node_type *, nodes, 32);

    node_type *node = _root;
    do {
        Assert(node);
        for (const Pair<_Key, void *>& it : node->_children)
            nodes.PushPOD(reinterpret_cast<node_type *>(it.second));

        node->_children.clear();

        allocator_type::destroy(node);
        allocator_type::deallocate(node, 1);
    } while (nodes.PopPOD(&node));

    _size = 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
