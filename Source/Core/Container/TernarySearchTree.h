#pragma once

#include "Core/Core.h"

#include "Core/Allocator/NodeBasedContainerAllocator.h"
#include "Core/Container/Stack.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Meta/PointerWFlags.h"

#include <algorithm>
#include <type_traits>

// String storage
// https://en.wikipedia.org/wiki/Ternary_search_tree

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define TERNARYSEARCHTREE(_DOMAIN, _KEY, _VALUE) \
    ::Core::TernarySearchTree<_KEY, _VALUE, ::Core::Meta::Less<_KEY>, ::Core::Meta::EqualTo<_KEY>, \
        NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, COMMA_PROTECT(::Core::TernarySearchNode<_KEY COMMA _VALUE>)) >
//----------------------------------------------------------------------------
#define TERNARYSEARCHTREE_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::TernarySearchTree<_KEY, _VALUE, ::Core::Meta::Less<_KEY>, ::Core::Meta::EqualTo<_KEY>, \
        THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, COMMA_PROTECT(::Core::TernarySearchNode<_KEY COMMA _VALUE>)) >
//----------------------------------------------------------------------------
#define TERNARYSEARCHSET(_DOMAIN, _KEY) TERNARYSEARCHTREE(_DOMAIN, _KEY, void)
#define TERNARYSEARCHSET_THREAD_LOCAL(_DOMAIN, _KEY) TERNARYSEARCHTREE_THREAD_LOCAL(_DOMAIN, _KEY, void)
//----------------------------------------------------------------------------
#define TERNARYSEARCHMAP(_DOMAIN, _KEY, _VALUE) TERNARYSEARCHTREE(_DOMAIN, _KEY, _VALUE)
#define TERNARYSEARCHMAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) TERNARYSEARCHTREE_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TernarySearchNode;
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator >
class TernarySearchTree;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TernarySearchNodeBase {
public:
    template <typename _K, typename _V, typename _L, typename _E, typename _A>
    friend class TernarySearchTree;

    typedef TernarySearchNode<_Key, _Value> node_type;

    explicit TernarySearchNodeBase(_Key&& rkey);
    explicit TernarySearchNodeBase(const _Key& key);
    ~TernarySearchNodeBase() {}

    TernarySearchNodeBase(const TernarySearchNodeBase& other) = delete;
    TernarySearchNodeBase& operator=(const TernarySearchNodeBase& other) = delete;

    const _Key& Key() const { return _key; }
    bool HasValue() const { return _parent_hasvalue.Flag0(); }

    const node_type* Left() const { return _left; }
    const node_type* Center() const { return _center; }
    const node_type* Right() const { return _right; }
    const node_type* Parent() const { return _parent_hasvalue.Get(); }

protected:
    void SetParent_(node_type* parent) { _parent_hasvalue.Set(parent); }
    void SetHasValue_(bool value) { Assert(value); _parent_hasvalue.SetFlag0(value); }

private:
    _Key _key;
    node_type* _left;
    node_type* _center;
    node_type* _right;
    Meta::PointerWFlags<node_type> _parent_hasvalue;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TernarySearchNode : public TernarySearchNodeBase<_Key, _Value> {
public:
    typedef TernarySearchNodeBase<_Key, _Value> parent_type;

    explicit TernarySearchNode(_Key&& rkey) : parent_type(std::move(rkey)) {}
    explicit TernarySearchNode(const _Key& key) : parent_type(key) {}
    ~TernarySearchNode() {}

    const _Value& Value() const { Assert(HasValue()); return _value; }

    void SetValue(_Value&& rvalue) { Assert(HasValue()); _value = std::move(rvalue); }
    void SetValue(const _Value& value) { Assert(HasValue()); _value = value; }

private:
    _Value _value;
};
//----------------------------------------------------------------------------
template <typename _Key>
class TernarySearchNode<_Key, void> : public TernarySearchNodeBase<_Key, void> {
public:
    typedef TernarySearchNodeBase<_Key, void> parent_type;

    explicit TernarySearchNode(_Key&& rkey) : parent_type(std::move(rkey)) {}
    explicit TernarySearchNode(const _Key& key) : parent_type(key) {}
    ~TernarySearchNode() {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Less = Meta::Less<_Key>,
    typename _EqualTo = Meta::EqualTo<_Key>,
    typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Container, TernarySearchNode<_Key COMMA _Value>)
>   class TernarySearchTree : _Allocator {
public:
    typedef _Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;

    typedef _Key key_type;
    typedef _Value value_type;

    typedef TernarySearchNode<_Key, _Value> node_type;
    typedef MemoryView<const _Key> sequence_type;

    typedef _Less less_functor;
    typedef _EqualTo equal_to_functor;

    typedef typename allocator_traits::pointer pointer;
    typedef typename allocator_traits::const_pointer const_pointer;

    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::difference_type difference_type;

    typedef const node_type* iterator;

    TernarySearchTree() : _root(nullptr), _size(0) {}
    ~TernarySearchTree();

    TernarySearchTree(const TernarySearchTree&) = delete;
    TernarySearchTree& operator =(const TernarySearchTree&) = delete;

    TernarySearchTree(TernarySearchTree&& rvalue);
    TernarySearchTree& operator =(TernarySearchTree&& rvalue) = delete;

    size_type size() const { return _size; }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }
    bool empty() const { return (0 == _size); }

    node_type* Root() { return _root; }
    const node_type* Root() const { return _root; }

    bool Insert_ReturnIfExists(node_type** pnode, const sequence_type& keys);
    node_type* Insert_AssertUnique(const sequence_type& keys);

    const node_type* Find(const sequence_type& keys, const node_type* hint = nullptr) const;
    bool Contains(const sequence_type& keys) const;

    void Clear();

    void Swap(TernarySearchTree& other);
    friend void swap(TernarySearchTree& lhs, TernarySearchTree& rhs) { lhs.Swap(rhs); }

private:
    node_type* _root;
    size_type _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/TernarySearchTree-inl.h"
