#pragma once

#include "Core.h"

#include "Container/Stack.h"
#include "Meta/AlignedStorage.h"
#include "Memory/MemoryView.h"
#include "Meta/PointerWFlags.h"

#include <algorithm>
#include <type_traits>

// FString storage
// https://en.wikipedia.org/wiki/Ternary_search_tree

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define TERNARYSEARCHTREE(_DOMAIN, _KEY, _VALUE) \
    ::PPE::TTernarySearchTree<_KEY, _VALUE, ::PPE::Meta::TLess<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, \
        BATCH_ALLOCATOR(_DOMAIN, COMMA_PROTECT(::PPE::TTernarySearchNode<_KEY COMMA _VALUE>)) >
//----------------------------------------------------------------------------
#define TERNARYSEARCHSET(_DOMAIN, _KEY) TERNARYSEARCHTREE(_DOMAIN, _KEY, void)
#define TERNARYSEARCHMAP(_DOMAIN, _KEY, _VALUE) TERNARYSEARCHTREE(_DOMAIN, _KEY, _VALUE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TTernarySearchNode;
template <typename _Key, typename _Value, typename _Less, typename _EqualTo, typename _Allocator >
class TTernarySearchTree;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TTernarySearchNodeBase {
public:
    template <typename _K, typename _V, typename _L, typename _E, typename _A>
    friend class TTernarySearchTree;

    typedef TTernarySearchNode<_Key, _Value> node_type;

    explicit TTernarySearchNodeBase(_Key&& rkey);
    explicit TTernarySearchNodeBase(const _Key& key);
    ~TTernarySearchNodeBase() = default;

    TTernarySearchNodeBase(const TTernarySearchNodeBase& other) = delete;
    TTernarySearchNodeBase& operator=(const TTernarySearchNodeBase& other) = delete;

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
    Meta::TPointerWFlags<node_type> _parent_hasvalue;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TTernarySearchNode : public TTernarySearchNodeBase<_Key, _Value> {
public:
    typedef TTernarySearchNodeBase<_Key, _Value> parent_type;

    using parent_type::HasValue;

    explicit TTernarySearchNode(_Key&& rkey) : parent_type(std::move(rkey)) {}
    explicit TTernarySearchNode(const _Key& key) : parent_type(key) {}
    ~TTernarySearchNode() = default;

    const _Value& Value() const { Assert(HasValue()); return _value; }

    void SetValue(_Value&& rvalue) { Assert(HasValue()); _value = std::move(rvalue); }
    void SetValue(const _Value& value) { Assert(HasValue()); _value = value; }

private:
    _Value _value;
};
//----------------------------------------------------------------------------
template <typename _Key>
class TTernarySearchNode<_Key, void> : public TTernarySearchNodeBase<_Key, void> {
public:
    typedef TTernarySearchNodeBase<_Key, void> parent_type;

    explicit TTernarySearchNode(_Key&& rkey) : parent_type(std::move(rkey)) {}
    explicit TTernarySearchNode(const _Key& key) : parent_type(key) {}
    ~TTernarySearchNode() = default;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Less = Meta::TLess<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Allocator = BATCH_ALLOCATOR(Container, TTernarySearchNode<_Key COMMA _Value>)
>   class TTernarySearchTree : _Allocator {
public:
    typedef _Allocator allocator_type;
    typedef TAllocatorTraits<allocator_type> allocator_traits;

    typedef _Key key_type;
    typedef _Value value_type;

    typedef TTernarySearchNode<_Key, _Value> node_type;
    typedef TMemoryView<const _Key> sequence_type;

    typedef _Less less_functor;
    typedef _EqualTo equal_to_functor;

    using pointer = Meta::TAddPointer<node_type>;
    using const_pointer = Meta::TAddPointer<Meta::TAddConst<node_type>>;
    using reference = Meta::TAddReference<node_type>;
    using const_reference = Meta::TAddReference<Meta::TAddConst<node_type>>;

    typedef size_t size_type;
    typedef const node_type* iterator;

    TTernarySearchTree() : _root(nullptr), _size(0) {}
    ~TTernarySearchTree();

    TTernarySearchTree(const TTernarySearchTree&) = delete;
    TTernarySearchTree& operator =(const TTernarySearchTree&) = delete;

    TTernarySearchTree(TTernarySearchTree&& rvalue) NOEXCEPT;
    TTernarySearchTree& operator =(TTernarySearchTree&& rvalue) NOEXCEPT = delete;

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

    void Swap(TTernarySearchTree& other);
    friend void swap(TTernarySearchTree& lhs, TTernarySearchTree& rhs) NOEXCEPT { lhs.Swap(rhs); }

private:
    node_type* _root;
    size_type _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/TernarySearchTree-inl.h"
