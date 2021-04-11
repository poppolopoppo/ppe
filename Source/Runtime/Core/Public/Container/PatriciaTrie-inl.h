#pragma once

#include "Container/PatriciaTrie.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu>
TPatriciaNodeBase<_Key, _Value, _InSitu>::TPatriciaNodeBase(const sequence_type& keys)
    : _size(0)
    , _hasValue(0)
    , _left(nullptr), _center(nullptr), _right(nullptr)
    , _parent(nullptr) {
    SetKeys_(keys);
    STATIC_ASSERT(_InSitu <= ((1<<7)-1)/* biggest number for 7 bits */);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu>
void TPatriciaNodeBase<_Key, _Value, _InSitu>::SetKeys_(const sequence_type& keys) {
    Assert(keys.size());
    Assert(keys.size() <= _InSitu);

    _size = checked_cast<u8>(keys.size());
    forrange(i, 0, _size)
        _keys[i] = keys[i];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::TPatriciaTrie()
    : _root(0), _size(0) {}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::~TPatriciaTrie() {
    Clear();
    Assert(nullptr == _root);
    Assert(0 == _size);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::TPatriciaTrie(TPatriciaTrie&& rvalue)
    :   allocator_type(std::move(rvalue))
    ,   _root(nullptr), _size(0) {
    std::swap(rvalue._root, _root);
    std::swap(rvalue._size, _size);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
bool TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::Insert_ReturnIfExists(node_type** pnode, const sequence_type& keys) {
    Assert(pnode);
    Assert(false == keys.empty());

    node_type* parent = nullptr;
    node_type** it = &_root;

    sequence_type ki = keys;
    while (ki.size()) {
        if (nullptr == *it) {
            *it = static_cast<node_type*>(allocator_traits::Allocate(*this, sizeof(node_type)).Data);
            Assert(nullptr != *it);

            const sequence_type ki0 = ki.CutBefore(std::min(ki.size(), _InSitu));

            allocator_traits::construct(*this, *it, ki0);
            (*it)->SetParent_(parent);

            if (ki.size() > _InSitu) {
                ki = ki.CutStartingAt(_InSitu);
                parent = *it;
                it = &(*it)->_center;
            }
            else {
                ki = sequence_type();
            }
        }
        else {
            const sequence_type kj = (*it)->Keys();

            const size_t kin = ki.size();
            const size_t kjn = kj.size();

            Assert(0 < kin);
            Assert(0 < kjn);

            size_t i = 0;
            for (; i < kin; ++i) {
                Assert(i <= _InSitu);
                if (i == kjn) {
                    parent = *it;
                    it = &(*it)->_center;

                    goto next_loop;
                }
                else if (not equal_to_functor()(ki[i], kj[i])) {
                    const _Key kji = kj[i];

                    node_type* const next = *it;
                    if (i > 0)
                        *it = parent = SplitNode_((*it), i);

                    if (less_functor()(ki[i], kji))
                        it = &next->_left;
                    else
                        it = &next->_right;

                    goto next_loop;
                }
            }

            if (i < kjn) {
                Assert(kin == i);

                node_type* const split = SplitNode_((*it), i);
                Assert(split->Parent() == parent);

                *it = split;
            }

        next_loop:
            ki = ki.CutStartingAt(i);
        }
    }

    Assert(*it);
    Assert((*it)->Keys().back() == keys.back());
    *pnode = *it;

    if ((*it)->HasValue()) {
        return true;
    }
    else {
        ++_size;
        (*it)->SetHasValue_(true);
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
auto TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::Insert_AssertUnique(const sequence_type& keys) -> node_type* {
    node_type* node = nullptr;
    if (true == Insert_ReturnIfExists(&node, keys))
        AssertNotReached();

    Assert(node);
    return node;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
auto TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::Find(const sequence_type& keys, const iterator* hint /* = nullptr */) const -> iterator {
    Assert(false == keys.empty());

    const node_type* it = _root;
    size_type i = 0;
    if (hint) {
        Assert(_root);
        if (hint->Relative == hint->Node->_size) {
            i = 0;
            it = hint->Node->_center;
        }
        else {
            i = hint->Relative;
            it = hint->Node;
        }
    }

    Assert(it);
    size_type j = 0;
    const size_t n = keys.size();

    while (j < n && nullptr != it) {
        const size_type m = it->_size;
        Assert(i < m);

        if (equal_to_functor()(keys[j], it->_keys[i])) {
            for (++i, ++j; i < m && j < n; ++i, ++j) {
                if (not equal_to_functor()(keys[j], it->_keys[i]))
                    return iterator{ nullptr, 0 };
            }

            if (j < n) {
                Assert(m == i);
                i = 0;
                it = it->_center;
            }
        }
        else if (less_functor()(keys[j], it->_keys[i])) {
            Assert(0 == i);
            it = it->_left;
        }
        else {
            Assert(0 == i);
            it = it->_right;
        }
    }

    /*
    while (j < n && nullptr != it) {
        if (i == it->_size) {
            Assert(0 < i);
            it = it->_center;
            i = 0;
        }
        else if (equal_to_functor()(keys[j], it->_keys[i])) {
            ++i;
            ++j;
        }
        else if (i > 0) { // key does not exist
            return iterator{ nullptr, 0 };
        }
        else if (less_functor()(keys[j], it->_keys[i])) {
            it = it->_left;
        }
        else {
            it = it->_right;
        }
    }
    */

    if (nullptr == it) {
        Assert(j < n);
        return iterator{ nullptr, 0 };
    }
    else {
        Assert(j == n);
        return iterator{ it, i };
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
bool TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::Contains(const sequence_type& keys) const {
    const iterator it = Find(keys);
    return it.HasValue();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
void TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::Optimize() {
    if (nullptr == _root) {
        Assert(0 == _size);
        return;
    }

    size_type merged = 0;
    size_type remaining = 0;

    STACKLOCAL_POD_STACK(node_type*, queue, 32);
    queue.Push(_root);

    node_type* node = nullptr;
    while (queue.Pop(&node)) {
        Assert(node);

        const bool mergeable =
            nullptr != node->_center &&
            node->_size + node->_center->_size <= _InSitu &&
            0 == node->_hasValue &&
            nullptr == node->_center->_left &&
            nullptr == node->_center->_right;

        if (mergeable) {
            Assert(node->_center->_parent == node);

            _Key* const pdst = node->_keys + node->_size;
            _Key* const psrc = node->_center->_keys;
            forrange(i, 0, node->_center->_size)
                pdst[i] = std::move(psrc[i]);

            node->_size += node->_center->_size;
            node->_hasValue = node->_center->_hasValue;

            node_type* const toDelete = node->_center;

            if (node->_center->_center) {
                Assert(node->_center->_center->_parent == node->_center);
                node->_center->_center->SetParent_(node);
                node->_center = node->_center->_center;
                Assert(node->_center->_parent == node);
            }
            else {
                node->_center = nullptr;
            }

            Assert(toDelete);

            allocator_traits::destroy(*this, toDelete);
            allocator_traits::deallocate(*this, toDelete, 1);

            queue.Push(node); // try to recursively optimize the node

            merged++;
        }
        else {
            if (node->_right)
                queue.Push(node->_right);
            if (node->_center)
                queue.Push(node->_center);
            if (node->_left)
                queue.Push(node->_left);

            remaining++;
        }
    }

    /*
    LOG(Info, L"[Patricia] Merged {0:6} nodes from a total of {1:6} ({2:6} remaining)",
        merged, merged+remaining, remaining );
        */
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
void TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::Clear() {
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
            _size--;
        }

        allocator_traits::destroy(*this, node);
        allocator_traits::deallocate(*this, node, 1);
    }

    Assert(0 == _size);
    _root = nullptr;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
void TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::Swap(TPatriciaTrie& other) {
    AssertRelease(static_cast<allocator_type&>(other) == static_cast<allocator_type&>(*this));
    std::swap(other._root, _root);
    std::swap(other._size, _size);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator>
auto TPatriciaTrie<_Key, _Value, _InSitu, _Less, _EqualTo, _Allocator>::SplitNode_(node_type* parent, size_t index) -> node_type* {
    Assert(parent);

    const sequence_type k0 = parent->Keys().CutBefore(index);
    const sequence_type k1 = parent->Keys().CutStartingAt(index);

    Assert(k0.size());
    Assert(k1.size());

    node_type* const split = allocator_traits::allocate(*this, 1);
    Assert(nullptr != split);

    allocator_traits::construct(*this, split, k0);
    split->_left = parent->_left;
    split->_center = parent;
    split->_right = parent->_right;
    split->SetParent_(parent->_parent);

    parent->SetKeys_(k1);
    parent->_left = nullptr;
    parent->_right = nullptr;
    parent->SetParent_(split);

    if (parent == _root) {
        Assert(nullptr == split->Parent());
        _root = split;
    }

    return split;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
