#pragma once

#include "Core.h"

#include "Memory/UniquePtr.h"
#include "IO/String.h"

#include "Container/PatriciaTrie.h"
#include "Container/TernarySearchTree.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define BURST_TRIE_INSITU 12 // no padding with chars
//----------------------------------------------------------------------------
#define BURST_TRIE(_DOMAIN, _CHAR, _VALUE, _CASE_SENSITIVE, _CAPACITY) \
    ::PPE::TBurstTrie<_CHAR, _VALUE, BURST_TRIE_INSITU, _CASE_SENSITIVE, _CAPACITY, \
        BATCH_ALLOCATOR(_DOMAIN, COMMA_PROTECT(::PPE::TPatriciaNode<_CHAR COMMA _VALUE COMMA BURST_TRIE_INSITU>)) >
//----------------------------------------------------------------------------
#define STRINGTRIE_SET(_DOMAIN, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE(_DOMAIN, char, void, _CASE_SENSITIVE, _CAPACITY)
#define STRINGTRIE_MAP(_DOMAIN, _VALUE, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE(_DOMAIN, char, _VALUE, _CASE_SENSITIVE, _CAPACITY)
//----------------------------------------------------------------------------
#define WSTRINGTRIE_SET(_DOMAIN, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE(_DOMAIN, wchar_t, void, _CASE_SENSITIVE, _CAPACITY)
#define WSTRINGTRIE_MAP(_DOMAIN, _VALUE, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE(_DOMAIN, wchar_t, _VALUE, _CASE_SENSITIVE, _CAPACITY)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char
,   typename _Value
,   size_t _InSitu = BURST_TRIE_INSITU
,   ECase _Sensitive = ECase::Sensitive
,   size_t _Capacity = 256
,   typename _Allocator = BATCH_ALLOCATOR(Container, TPatriciaNode<_Char COMMA _Value COMMA _InSitu>)
>   class TBurstTrie {
public:
    typedef TCharCase<_Char, _Sensitive> case_functor;
    typedef TCharLess<_Char, _Sensitive> less_functor;
    typedef TCharEqualTo<_Char, _Sensitive> equal_to_functor;

#if 1
    typedef TPatriciaTrie<_Char, _Value, BURST_TRIE_INSITU, less_functor, equal_to_functor, _Allocator> tree_type;
#else
    typedef TTernarySearchTree<_Char, _Value, less_functor, equal_to_functor, _Allocator> tree_type;
#endif

    typedef typename tree_type::node_type node_type;
    typedef typename tree_type::sequence_type sequence_type;
    typedef typename tree_type::size_type size_type;
    typedef typename tree_type::iterator iterator;

    //STATIC_ASSERT(Meta::IsPow2(_Capacity));
    static constexpr size_type Capacity = _Capacity;

    struct query_t {
        size_type Hash = 0;
        iterator It = iterator{ nullptr };
    };

    TBurstTrie() : _size(0) {}
    ~TBurstTrie() { Clear(); }

    TBurstTrie(const TBurstTrie&) = delete;
    TBurstTrie& operator =(const TBurstTrie&) = delete;

    size_type size() const { return _size; }
    bool empty() const { return (0 == _size); }

    bool Insert_ReturnIfExists(node_type** pnode, const sequence_type& keys);
    node_type* Insert_AssertUnique(const sequence_type& keys);

    iterator Find(const sequence_type& keys) const;
    query_t Find(const sequence_type& keys, const query_t* phint) const;
    bool Contains(const sequence_type& keys) const;

    void Optimize();
    void Clear();

private:
    size_type Hash_(const _Char key) const { return size_type(case_functor()(key)) % Capacity; }

    tree_type& Tree_(size_type hash) {
        Assert(hash < Capacity);
        return _root[hash];
    }

    const tree_type& Tree_(size_type hash) const {
        Assert(hash < Capacity);
        return _root[hash];
    }

    tree_type _root[Capacity];
    size_type _size;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, size_t _InSitu, ECase _Sensitive, size_t _Capacity, typename _Allocator>
bool TBurstTrie<_Char, _Value, _InSitu, _Sensitive, _Capacity, _Allocator>::Insert_ReturnIfExists(node_type** pnode, const sequence_type& keys) {
    if (false == Tree_(Hash_(keys.front())).Insert_ReturnIfExists(pnode, keys)) {
        ++_size;
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, size_t _InSitu, ECase _Sensitive, size_t _Capacity, typename _Allocator>
auto TBurstTrie<_Char, _Value, _InSitu, _Sensitive, _Capacity, _Allocator>::Insert_AssertUnique(const sequence_type& keys) -> node_type* {
    ++_size;
    return Tree_(Hash_(keys.front())).Insert_AssertUnique(keys);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, size_t _InSitu, ECase _Sensitive, size_t _Capacity, typename _Allocator>
auto TBurstTrie<_Char, _Value, _InSitu, _Sensitive, _Capacity, _Allocator>::Find(const sequence_type& keys) const -> iterator {
    return Tree_(Hash_(keys.front())).Find(keys);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, size_t _InSitu, ECase _Sensitive, size_t _Capacity, typename _Allocator>
auto TBurstTrie<_Char, _Value, _InSitu, _Sensitive, _Capacity, _Allocator>::Find(const sequence_type& keys, const query_t* phint) const -> query_t {
    query_t result;
    if (phint) {
        Assert(phint->It);
        result.Hash = phint->Hash;
        result.It = Tree_(result.Hash).Find(keys, &phint->It);
    }
    else {
        result.Hash = Hash_(keys.front());
        result.It = Tree_(result.Hash).Find(keys);
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, size_t _InSitu, ECase _Sensitive, size_t _Capacity, typename _Allocator>
bool TBurstTrie<_Char, _Value, _InSitu, _Sensitive, _Capacity, _Allocator>::Contains(const sequence_type& keys) const {
    return Tree_(Hash_(keys.front())).Contains(keys);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, size_t _InSitu, ECase _Sensitive, size_t _Capacity, typename _Allocator>
void TBurstTrie<_Char, _Value, _InSitu, _Sensitive, _Capacity, _Allocator>::Optimize() {
    for (tree_type& tree : _root)
        tree.Optimize();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, size_t _InSitu, ECase _Sensitive, size_t _Capacity, typename _Allocator>
void TBurstTrie<_Char, _Value, _InSitu, _Sensitive, _Capacity, _Allocator>::Clear() {
    for (tree_type& tree : _root) {
        Assert(_size >= tree.size());
        _size -= tree.size();
        tree.Clear();
    }
    Assert(0 == _size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
