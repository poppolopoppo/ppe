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
#define PATRICIATRIE(_DOMAIN, _KEY, _VALUE) \
    ::Core::PatriciaTrie<_KEY, _VALUE, ::Core::Meta::Less<_KEY>, ::Core::Meta::EqualTo<_KEY>, \
        NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, COMMA_PROTECT(::Core::PatriciaNode<_KEY COMMA _VALUE>)) >
//----------------------------------------------------------------------------
#define PATRICIATRIE_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::PatriciaTrie<_KEY, _VALUE, ::Core::Meta::Less<_KEY>, ::Core::Meta::EqualTo<_KEY>, \
        THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, COMMA_PROTECT(::Core::PatriciaNode<_KEY COMMA _VALUE>)) >
//----------------------------------------------------------------------------
#define PATRICIASET(_DOMAIN, _KEY) PATRICIATRIE(_DOMAIN, _KEY, void)
#define PATRICIASET_THREAD_LOCAL(_DOMAIN, _KEY) PATRICIATRIE_THREAD_LOCAL(_DOMAIN, _KEY, void)
//----------------------------------------------------------------------------
#define PATRICIAMAP(_DOMAIN, _KEY, _VALUE) PATRICIATRIE(_DOMAIN, _KEY, _VALUE)
#define PATRICIAMAP_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) PATRICIATRIE_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu>
class PatriciaNode;
template <typename _Key, typename _Value, size_t _InSitu, typename _Less, typename _EqualTo, typename _Allocator >
class PatriciaTrie;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu>
class PatriciaNodeBase {
public:
    template <typename _K, typename _V, size_t _I, typename _L, typename _E, typename _A>
    friend class PatriciaTrie;

    typedef PatriciaNode<_Key, _Value, _InSitu> node_type;
    typedef MemoryView<const _Key> sequence_type;

    static constexpr size_t InSitu = _InSitu;

    explicit PatriciaNodeBase(const sequence_type& keys);
    ~PatriciaNodeBase() {}

    PatriciaNodeBase(const PatriciaNodeBase& other) = delete;
    PatriciaNodeBase& operator=(const PatriciaNodeBase& other) = delete;

    sequence_type Keys() const { return sequence_type(_keys, _size); }

    bool HasValue() const { return (_hasValue ? true : false); }

    const node_type* Left() const { return _left; }
    const node_type* Center() const { return _center; }
    const node_type* Right() const { return _right; }

    const node_type* Parent() const { return _parent; }

protected:
    void SetKeys_(const sequence_type& keys);
    void SetParent_(node_type* parent) { _parent = parent; }
    void SetHasValue_(bool value) { _hasValue = (value ? 1 : 0); }

private:
    _Key _keys[_InSitu];
    u8 _size        : 7;
    u8 _hasValue    : 1;

    node_type* _left;
    node_type* _center;
    node_type* _right;

    node_type* _parent;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSitu>
class PatriciaNode : public PatriciaNodeBase<_Key, _Value, _InSitu> {
public:
    typedef PatriciaNodeBase<_Key, _Value, _InSitu> parent_type;

    using typename parent_type::node_type;
    using typename parent_type::sequence_type;

    explicit PatriciaNode(const sequence_type& keys) : parent_type(keys) {}

    const _Value& Value() const { Assert(HasValue()); return _value; }

    void SetValue(_Value&& rvalue) { Assert(HasValue()); _value = std::move(rvalue); }
    void SetValue(const _Value& value) { Assert(HasValue()); _value = value; }

private:
    _Value _value;
};
//----------------------------------------------------------------------------
template <typename _Key, size_t _InSitu>
class PatriciaNode<_Key, void, _InSitu> : public PatriciaNodeBase<_Key, void, _InSitu> {
public:
    typedef PatriciaNodeBase<_Key, void, _InSitu> parent_type;

    using typename parent_type::node_type;
    using typename parent_type::sequence_type;

    explicit PatriciaNode(const sequence_type& keys) : parent_type(keys) {}

    void MoveValueFrom(node_type& other) {
        Assert(false == HasValue());
        SetHasValue_(other.HasValue());
        other.SetHasValue_(false);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Value
,   size_t _InSitu
,   typename _Less = Meta::Less<_Key>
,   typename _EqualTo = Meta::EqualTo<_Key>
,   typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Container, PatriciaNode<_Key COMMA _Value COMMA _InSitu>)
>   class PatriciaTrie : _Allocator {
public:
    typedef _Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;

    typedef _Key key_type;
    typedef _Value value_type;

    typedef PatriciaNode<_Key, _Value, _InSitu> node_type;
    typedef typename node_type::sequence_type sequence_type;

    typedef _Less less_functor;
    typedef _EqualTo equal_to_functor;

    typedef typename allocator_traits::pointer pointer;
    typedef typename allocator_traits::const_pointer const_pointer;

    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::difference_type difference_type;

    struct iterator {
        const node_type* Node;
        size_type Relative;
        operator const void* () const { return Node; }
        bool HasValue() const {
            return (nullptr != Node &&
                    Node->HasValue() &&
                    Relative == Node->_size ); }
    };

    PatriciaTrie();
    ~PatriciaTrie();

    PatriciaTrie(const PatriciaTrie&) = delete;
    PatriciaTrie& operator =(const PatriciaTrie&) = delete;

    PatriciaTrie(PatriciaTrie&& rvalue);
    PatriciaTrie& operator =(PatriciaTrie&& rvalue) = delete;

    size_type size() const { return _size; }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }
    bool empty() const { return (0 == _size); }

    node_type* Root() { return _root; }
    const node_type* Root() const { return _root; }

    bool Insert_ReturnIfExists(node_type** pnode, const sequence_type& keys);
    node_type* Insert_AssertUnique(const sequence_type& keys);

    iterator Find(const sequence_type& keys, const iterator* hint = nullptr) const;
    bool Contains(const sequence_type& keys) const;

    void Optimize();
    void Clear();

    void Swap(PatriciaTrie& other);
    friend void swap(PatriciaTrie& lhs, PatriciaTrie& rhs) { lhs.Swap(rhs); }

private:
    node_type* SplitNode_(node_type* parent, size_t index);

    node_type* _root;
    size_type _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/PatriciaTrie-inl.h"
