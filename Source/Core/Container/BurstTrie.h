#pragma once

#include "Core/Core.h"

#include "Core/Container/TernarySearchTree.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define BURST_TRIE(_DOMAIN, _CHAR, _VALUE, _CASE_SENSITIVE, _CAPACITY) \
    ::Core::BurstTrie<_CHAR, _VALUE, _CASE_SENSITIVE, _CAPACITY, \
        NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, COMMA_PROTECT(::Core::TernarySearchNode<_CHAR COMMA _VALUE>)) >
//----------------------------------------------------------------------------
#define BURST_TRIE_THREAD_LOCAL(_DOMAIN, _CHAR, _VALUE, _CASE_SENSITIVE, _CAPACITY) \
    ::Core::BurstTrie<_CHAR, _VALUE, _CASE_SENSITIVE, _CAPACITY, \
        THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_DOMAIN, COMMA_PROTECT(::Core::TernarySearchNode<_CHAR COMMA _VALUE>)) >
//----------------------------------------------------------------------------
#define STRINGTRIE_SET(_DOMAIN, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE(_DOMAIN, char, void, _CASE_SENSITIVE, _CAPACITY)
#define STRINGTRIE_SET_THREAD_LOCAL(_DOMAIN, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE_THREAD_LOCAL(_DOMAIN, char, void, _CASE_SENSITIVE, _CAPACITY)
//----------------------------------------------------------------------------
#define WSTRINGTRIE_SET(_DOMAIN, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE(_DOMAIN, wchar_t, void, _CASE_SENSITIVE, _CAPACITY)
#define WSTRINGTRIE_SET_THREAD_LOCAL(_DOMAIN, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE_THREAD_LOCAL(_DOMAIN, wchar_t, void, _CASE_SENSITIVE, _CAPACITY)
//----------------------------------------------------------------------------
#define STRINGTRIE_MAP(_DOMAIN, _VALUE, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE(_DOMAIN, char, _VALUE, _CASE_SENSITIVE, _CAPACITY)
#define STRINGTRIE_MAP_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE_THREAD_LOCAL(_DOMAIN, char, _VALUE, _CASE_SENSITIVE, _CAPACITY)
//----------------------------------------------------------------------------
#define WSTRINGTRIE_MAP(_DOMAIN, _VALUE, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE(_DOMAIN, wchar_t, _VALUE, _CASE_SENSITIVE, _CAPACITY)
#define WSTRINGTRIE_MAP_THREAD_LOCAL(_DOMAIN, _VALUE, _CASE_SENSITIVE, _CAPACITY) BURST_TRIE_THREAD_LOCAL(_DOMAIN, wchar_t, _VALUE, _CASE_SENSITIVE, _CAPACITY)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Value,
    CaseSensitive _CaseSensitive = CaseSensitive::True,
    size_t _Capacity = 256,
    typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Container, TernarySearchNode<_Key COMMA _Value>)
>   class BurstTrie {
public:
    typedef CharCase<_Char, _CaseSensitive> case_functor;
    typedef CharLess<_Char, _CaseSensitive> less_functor;
    typedef CharEqualTo<_Char, _CaseSensitive> equal_to_functor;

    typedef TernarySearchTree<_Char, _Value, less_functor, equal_to_functor, _Allocator> tree_type;

    typedef typename tree_type::node_type node_type;
    typedef typename tree_type::sequence_type sequence_type;
    typedef typename tree_type::size_type size_type;

    STATIC_ASSERT(IS_POW2(_Capacity));
    static constexpr size_type Capacity = _Capacity;

    struct query_t {
        size_type Hash = 0;
        const node_type* Node = nullptr;
    };

    BurstTrie() : _size(0) {}
    ~BurstTrie() { Clear(); }

    BurstTrie(const BurstTrie&) = delete;
    BurstTrie& operator =(const BurstTrie&) = delete;

    size_type size() const { return _size; }
    bool empty() const { return (0 == _size); }

    bool Insert_ReturnIfExists(node_type** pnode, const sequence_type& keys);
    node_type* Insert_AssertUnique(const sequence_type& keys);

    const node_type* Find(const sequence_type& keys) const;
    query_t Find(const sequence_type& keys, const query_t* phint) const;
    bool Contains(const sequence_type& keys) const;

    void Clear();

private:
    size_type Hash_(const _Char key) const { return size_type(case_functor()(key)) & (Capacity - 1); }

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
template <typename _Char, typename _Value, CaseSensitive _CaseSensitive, size_t _Capacity, typename _Allocator>
bool BurstTrie<_Char, _Value, _CaseSensitive, _Capacity, _Allocator>::Insert_ReturnIfExists(node_type** pnode, const sequence_type& keys) {
    if (false == Tree_(Hash_(keys.front())).Insert_ReturnIfExists(pnode, keys)) {
        ++_size;
        return false:
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, CaseSensitive _CaseSensitive, size_t _Capacity, typename _Allocator>
auto BurstTrie<_Char, _Value, _CaseSensitive, _Capacity, _Allocator>::Insert_AssertUnique(const sequence_type& keys) -> node_type* {
    ++_size;
    return Tree_(Hash_(keys.front())).Insert_AssertUnique(keys);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, CaseSensitive _CaseSensitive, size_t _Capacity, typename _Allocator>
auto BurstTrie<_Char, _Value, _CaseSensitive, _Capacity, _Allocator>::Find(const sequence_type& keys) const -> const node_type* {
    return Tree_(Hash_(keys.front())).Find(keys);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, CaseSensitive _CaseSensitive, size_t _Capacity, typename _Allocator>
auto BurstTrie<_Char, _Value, _CaseSensitive, _Capacity, _Allocator>::Find(const sequence_type& keys, const query_t* phint) const -> query_t {
    query_t result;
    if (phint) {
        Assert(phint->Node);
        result.Hash = phint->Hash;
        result.Node = Tree_(result.Hash).Find(keys, phint->Node);
    }
    else {
        result.Hash = Hash_(keys.front());
        result.Node = Tree_(result.Hash).Find(keys);
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, CaseSensitive _CaseSensitive, size_t _Capacity, typename _Allocator>
bool BurstTrie<_Char, _Value, _CaseSensitive, _Capacity, _Allocator>::Contains(const sequence_type& keys) const {
    return Tree_(keys.front()).Contains(keys);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Value, CaseSensitive _CaseSensitive, size_t _Capacity, typename _Allocator>
void BurstTrie<_Char, _Value, _CaseSensitive, _Capacity, _Allocator>::Clear() {
    forrange(i, 0, Capacity) {
        Assert(_size >= _root[i].size());
        _size -= _root[i].size();
        _root[i].Clear();
    }
    Assert(0 == _size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
