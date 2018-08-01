#pragma once

#include "Container/TernarySearchTree.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
TTernarySearchNodeBase<_Key, _Value>::TTernarySearchNodeBase(_Key&& rkey)
    : _key(std::move(rkey))
    , _left(nullptr), _center(nullptr), _right(nullptr) {
    _parent_hasvalue.Reset(nullptr, false);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
TTernarySearchNodeBase<_Key, _Value>::TTernarySearchNodeBase(const _Key& key)
    : _key(key)
    , _left(nullptr), _center(nullptr), _right(nullptr) {
    _parent_hasvalue.Reset(nullptr, false);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator>
TTernarySearchTree<_Key, _Value, _Less, _EqualTo, _Allocator>::~TTernarySearchTree() {
    Clear();
    Assert(nullptr == _root);
    Assert(0 == _size);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator>
TTernarySearchTree<_Key, _Value, _Less, _EqualTo, _Allocator>::TTernarySearchTree(TTernarySearchTree&& rvalue)
    :   allocator_type(std::move(rvalue))
    ,   _root(nullptr), _size(0) {
    std::swap(rvalue._root, _root);
    std::swap(rvalue._size, _size);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator>
bool TTernarySearchTree<_Key, _Value, _Less, _EqualTo, _Allocator>::Insert_ReturnIfExists(node_type** pnode, const sequence_type& keys) {
    Assert(pnode);
    Assert(false == keys.empty());

    const size_type n = keys.size();

    node_type* parent = _root;
    node_type** it = &_root;

    forrange(i, 0, n) {
        const _Key& key = keys[i];
        do {
            if (nullptr == *it) {
                *it = allocator_traits::allocate(*this, 1);
                Assert(nullptr != *it);
                allocator_traits::construct(*this, *it, key);
                (*it)->SetParent_(parent);
            }

            if (equal_to_functor()(key, (*it)->_key)) {
                parent = *it;
                it = &(*it)->_center;
                break;
            }
            else if (less_functor()(key, (*it)->_key)) {
                it = &(*it)->_left;
            }
            else {
                it = &(*it)->_right;
            }
        }
        while (true);
    }

    Assert(parent);
    Assert(parent->_key == keys.back());
    *pnode = parent;

    if (parent->HasValue()) {
        return true;
    }
    else {
        ++_size;
        parent->SetHasValue_(true);
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator>
auto TTernarySearchTree<_Key, _Value, _Less, _EqualTo, _Allocator>::Insert_AssertUnique(const sequence_type& keys) -> node_type* {
    node_type* node = nullptr;
    if (true == Insert_ReturnIfExists(&node, keys))
        AssertNotReached();

    Assert(node);
    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator>
auto TTernarySearchTree<_Key, _Value, _Less, _EqualTo, _Allocator>::Find(const sequence_type& keys, const node_type* hint /* = nullptr */) const -> const node_type* {
    Assert(false == keys.empty());

    const size_type n = keys.size();
    const node_type* it = (nullptr == hint ? _root : hint->_center);
    const node_type* parent = nullptr;

    forrange(i, 0, n) {
        const _Key& key = keys[i];
        while (nullptr != it) {
            if (equal_to_functor()(key, it->_key)) {
                parent = it;
                it = it->_center;
                break;
            }
            else if (less_functor()(key, it->_key)) {
                it = it->_left;
            }
            else {
                it = it->_right;
            }
        }
    }

    return parent;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator>
bool TTernarySearchTree<_Key, _Value, _Less, _EqualTo, _Allocator>::Contains(const sequence_type& keys) const {
    const node_type* node = Find(keys);
    return (nullptr != node &&
            node->HasValue() &&
            equal_to_functor()(node->_key, keys.back()) );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator>
void TTernarySearchTree<_Key, _Value, _Less, _EqualTo, _Allocator>::Clear() {
    if (nullptr == _root) {
        Assert(0 == _size);
        return;
    }

    STACKLOCAL_POD_STACK(node_type*, queue, 32);
    queue.Push(_root);

    node_type* node = nullptr;
    while (queue.Pop(&node)) {
        Assert(node);

        if (node->_left)
            queue.Push(node->_left);
        if (node->_center)
            queue.Push(node->_center);
        if (node->_right)
            queue.Push(node->_right);

        if (node->HasValue()) {
            Assert(0 < _size);
            --_size;
        }

        allocator_traits::destroy(*this, node);
        allocator_traits::deallocate(*this, node, 1);
    }

    Assert(0 == _size);
    _root = nullptr;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator>
void TTernarySearchTree<_Key, _Value, _Less, _EqualTo, _Allocator>::Swap(TTernarySearchTree& other) {
    AssertRelease(static_cast<allocator_type&>(other) == static_cast<allocator_type&>(*this));
    std::swap(other._root, _root);
    std::swap(other._size, _size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
