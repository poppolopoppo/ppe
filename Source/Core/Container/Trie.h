#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/NodeBasedContainerAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define TRIE(_DOMAIN, _KEY, _VALUE, _InSituCount) \
    ::Core::Trie<_KEY, _VALUE, _InSituCount, std::equal_to<_KEY>, ALLOCATOR(_DOMAIN, ::Core::Pair<COMMA_PROTECT(_KEY) COMMA COMMA_PROTECT(_VALUE)>) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _InSituCount, typename _EqualTo, typename _Allocator>
class Trie;
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    size_t   _InSituCount,
    typename _EqualTo = EqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container, Pair<_Key COMMA _Value>)
>   class TrieNode {
public:
    friend class Trie<_Key, _Value, _InSituCount, _EqualTo, _Allocator>;

    typedef AssociativeVector<
        _Key
    ,   void *
    ,   _EqualTo
    ,   VectorInSitu< Pair<_Key, void *>, _InSituCount, _Allocator >
    >   node_index_map;

    TrieNode();
    ~TrieNode();

    size_t size() const { return _children.size(); }
    bool empty() const { return _children.empty(); }

    const _Value& Value() const { return _value; }

    void SetValue(_Value&& rvalue) { _value = std::move(rvalue); }
    void SetValue(const _Value& value) { _value = value; }

    template <typename _Arg0, typename... _Args>
    void Emplace(_Arg0&& arg0, _Args&&... args) {
       _value = _Value(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }

    const node_index_map& Children() const { return _children; }

    bool Find(const _Key& key, TrieNode **node);
    bool Find(const _Key& key, const TrieNode **node) const;

private:
    _Value _value;
    node_index_map _children;
};
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    size_t   _InSituCount = 3,
    typename _EqualTo = EqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container, Pair<_Key COMMA _Value>)
>   class Trie : NODEBASED_CONTAINER_ALLOCATOR(Container, COMMA_PROTECT(TrieNode<_Key COMMA _Value COMMA _InSituCount COMMA _EqualTo COMMA _Allocator>)) {
public:
    STATIC_ASSERT(_InSituCount > 0);
    typedef TrieNode<_Key, _Value, _InSituCount, _EqualTo, _Allocator> node_type;
    typedef NODEBASED_CONTAINER_ALLOCATOR(Container, node_type) allocator_type;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    Trie();
    ~Trie();

    Trie(const Trie&) = delete;
    Trie& operator =(const Trie&) = delete;

    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }

    node_type *Root() { return _root; }
    const node_type *Root() const { return _root; }

    const node_type *Insert_AssertUnique(const MemoryView<const _Key>& keys, _Value&& rvalue);
    const node_type *Insert_AssertUnique(const MemoryView<const _Key>& keys, const _Value& value);

    const node_type *Insert_OverwriteIFN(const MemoryView<const _Key>& keys, _Value&& rvalue);
    const node_type *Insert_OverwriteIFN(const MemoryView<const _Key>& keys, const _Value& value);

    template <typename _Arg0, typename... _Args>
    const node_type *Emplace_AssertUnique(_Arg0&& arg0, _Args&&... args) {
        _Value rvalue = _Value(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        return Insert_AssertUnique(std::move(rvalue));
    }

    template <typename _Arg0, typename... _Args>
    const node_type *Emplace_OverwriteIFN(_Arg0&& arg0, _Args&&... args) {
        _Value rvalue = _Value(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        return Insert_OverwriteIFN(std::move(rvalue));
    }

    size_t Follow_ReturnDepth(const node_type **pLastNode, const MemoryView<const _Key>& keys) const;

    template <typename _Functor>
    size_t EachNode_BreadthFirst(_Functor&& functor) const;
    template <typename _Functor>
    size_t EachNode_DepthFirst(_Functor&& functor) const;

    void Clear();

private:
    node_type *CreatePathIFN_(const MemoryView<const _Key>& keys);

    node_type *_root;
    size_type _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Trie-inl.h"
