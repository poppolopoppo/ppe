#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Hash.h"
#include "Core/Container/Pair.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>

// http://codecapsule.com/2013/11/17/robin-hood-hashing-backward-shift-deletion/
// https://github.com/goossaert/hashmap/blob/master/backshift_hashmap.cc
// http://codecapsule.com/2013/08/11/hopscotch-hashing/
// http://www.sebastiansylvan.com/post/robin-hood-hashing-should-be-your-default-hash-table-implementation/
// http://stackoverflow.com/questions/245878/how-do-i-choose-between-a-hash-table-and-a-trie-prefix-tree
// http://www.ilikebigbits.com/blog/2016/8/28/designing-a-fast-hash-table

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Key, typename _Value>
struct THashMapTraits_ {
    typedef _Key key_type;
    typedef _Value mapped_type;
    typedef TPair<_Key, _Value> value_type;
    typedef TPair<Meta::TAddConst<_Key>, _Value> public_type;
    typedef Meta::TAddReference<mapped_type> mapped_reference;
    typedef Meta::TAddReference<const mapped_type> mapped_reference_const;
    static key_type& Key(value_type& value) { return value.first; }
    static const key_type& Key(const value_type& value) { return value.first; }
    static mapped_type& Value(value_type& value) { return value.second; }
    static const mapped_type& Value(const value_type& value) { return value.second; }
    static mapped_type& Value(public_type& value) { return value.second; }
    static const mapped_type& Value(const public_type& value) { return value.second; }
    static value_type Make(const _Key& key) { return value_type(key, _Value()); }
    static value_type Make(_Key&& rkey) { return value_type(std::move(rkey), _Value()); }
    static value_type Make(const _Key& key, const _Value& value) { return value_type(key, value); }
    static value_type Make(const _Key& key, _Value&& rvalue) { return value_type(key, std::move(rvalue)); }
    static value_type Make(_Key&& rkey, _Value&& rvalue) { return value_type(std::move(rkey), std::move(rvalue)); }
    template <typename _It> static TKeyIterator<_It> MakeKeyIterator(_It&& it) { return Core::MakeKeyIterator(std::move(it)); }
    template <typename _It> static TValueIterator<_It> MakeValueIterator(_It&& it) { return Core::MakeValueIterator(std::move(it)); }
};
template <typename _Key>
struct THashSetTraits_ {
    typedef _Key key_type;
    typedef _Key value_type;
    typedef _Key mapped_type;
    typedef Meta::TAddConst<_Key> public_type;
    typedef Meta::TAddReference<public_type> mapped_reference_const;
    typedef mapped_reference_const mapped_reference;
    static _Key& Key(value_type& value) { return value; }
    static const _Key& Key(const value_type& value) { return value; }
    static mapped_type& Value(value_type& value) { return value; }
    static const mapped_type& Value(const value_type& value) { return value; }
    template <typename... _Args>
    static value_type Make(_Args&&... args) { return value_type(std::forward<_Args>(args)...); }
    template <typename _It> static const _It& MakeKeyIterator(const _It& it) { return it; }
    template <typename _It> static const _It& MakeValueIterator(const _It& it) { return it; }
};
} //!details
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
class TBasicHashTable : _Allocator {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <typename T>
    class TIterator_ : public std::iterator<std::forward_iterator_tag, T, difference_type, Meta::TAddPointer<T>, Meta::TAddReference<T> > {
    public:
        typedef std::iterator<std::forward_iterator_tag, T, difference_type, Meta::TAddPointer<T>, Meta::TAddReference<T> > parent_type;

        using typename parent_type::value_type;
        using typename parent_type::reference;
        using typename parent_type::pointer;
        using typename parent_type::difference_type;
        using typename parent_type::iterator_category;

        TIterator_() NOEXCEPT : _m(nullptr), _p(nullptr) {}
        TIterator_(const TBasicHashTable& m, pointer p) : _m(&m), _p(p) {}

        template <typename U>
        TIterator_(const TIterator_<U>& other) : _m(other.map()), _p(other.data()) {}
        template <typename U>
        TIterator_& operator =(const TIterator_<U>& other) {
            _m = other.map();
            _p = other.data();
            return *this;
        }

        const TBasicHashTable* map() const { return _m; }
        pointer data() const { return _p; }

        TIterator_& operator++() /* prefix */ { return GotoNextBucket_(); }
        TIterator_ operator++(int) /* postfix */ { return TIterator_(*this).GotoNextBucket_(); }

        reference operator*() const { Assert(_p); return *_p; }
        pointer operator->() const { Assert(_p); return _p; }

        void swap(TIterator_& other) {
            std::swap(_m, other._m);
            std::swap(_p, other._p);
        }
        inline friend void swap(TIterator_& lhs, TIterator_& rhs) { lhs.swap(rhs); }

        bool AliasesToContainer(const TBasicHashTable& m) const {
            return (_m == &m && m.AliasesToContainer(reinterpret_cast<const_pointer>(_p)));
        }

        template <typename U>
        bool operator ==(const TIterator_<U>& other) const { Assert(_m == other._m); return (_p == other.data()); }
        template <typename U>
        bool operator !=(const TIterator_<U>& other) const { Assert(_m == other._m); return (_p != other.data()); }

    private:
        TIterator_& GotoNextBucket_();

        const TBasicHashTable* _m;
        pointer _p;
    };

    typedef _Traits table_traits;

    typedef typename table_traits::key_type key_type;
    typedef typename table_traits::mapped_type mapped_type;
    typedef typename table_traits::value_type value_type;
    typedef typename table_traits::public_type public_type;

    typedef typename table_traits::mapped_reference mapped_reference;
    typedef typename table_traits::mapped_reference_const mapped_reference_const;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef _Hasher hasher;
    typedef _EqualTo key_equal;

    typedef _Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;

    typedef TIterator_<public_type> iterator;
    typedef TIterator_<Meta::TAddConst<public_type>> const_iterator;

    TBasicHashTable() NOEXCEPT { STATIC_ASSERT(sizeof(*this) == sizeof(u32) * 2 + sizeof(intptr_t)); }
    ~TBasicHashTable() { Assert(CheckInvariants()); clear_ReleaseMemory(); }

    explicit TBasicHashTable(allocator_type&& alloc) : allocator_type(std::move(alloc)) {}
    explicit TBasicHashTable(const allocator_type& alloc) : allocator_type(alloc) {}

    TBasicHashTable(hasher&& hash, key_equal&& equalto) : _data(std::move(hash), std::move(equalto)) {}
    TBasicHashTable(hasher&& hash, key_equal&& equalto, allocator_type&& alloc) : allocator_type(std::move(alloc)), _data(std::move(hash), std::move(equalto)) {}

    explicit TBasicHashTable(size_type capacity) : TBasicHashTable() { reserve(capacity); }
    TBasicHashTable(size_type capacity, const allocator_type& alloc) : TBasicHashTable(alloc) { reserve(capacity); }

    TBasicHashTable(const TBasicHashTable& other) : TBasicHashTable(allocator_traits::select_on_container_copy_construction(other)) { assign(other.begin(), other.end()); }
    TBasicHashTable(const TBasicHashTable& other, const allocator_type& alloc) : TBasicHashTable(alloc) { assign(other.begin(), other.end()); }
    TBasicHashTable& operator=(const TBasicHashTable& other);

    TBasicHashTable(TBasicHashTable&& rvalue) NOEXCEPT : TBasicHashTable(static_cast<allocator_type&&>(rvalue)) { assign(std::move(rvalue)); }
    TBasicHashTable(TBasicHashTable&& rvalue, const allocator_type& alloc) NOEXCEPT : TBasicHashTable(alloc) { assign_rvalue_(std::move(rvalue), std::false_type()); }
    TBasicHashTable& operator=(TBasicHashTable&& rvalue) NOEXCEPT;

    TBasicHashTable(std::initializer_list<value_type> ilist) : TBasicHashTable() { assign(ilist.begin(), ilist.end()); }
    TBasicHashTable(std::initializer_list<value_type> ilist, const allocator_type& alloc) : TBasicHashTable(alloc) { assign(ilist.begin(), ilist.end()); }
    TBasicHashTable& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    size_type capacity() const { return (_data.CapacityM1 + 1); }
    bool empty() const { return (0 == _data.Size); }
    size_type size() const { return size_type(_data.Size); }

    size_type bucket_count() const { return capacity(); }
    size_type max_bucket_count() const { return Min(size_type(1ull << 24) - 1, size_type(allocator_traits::max_size(*this) / sizeof(value_type))); }

    float load_factor() const;
    size_type max_probe_dist() const { return _data.MaxProbeDist; }

    iterator begin() { return MakeIterator_(_data.NextBucket(0)); }
    iterator end() { return MakeIterator_(capacity()); }

    const_iterator begin() const { return MakeIterator_(_data.NextBucket(0)); }
    const_iterator end() const { return MakeIterator_(capacity()); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    auto Keys() { return MakeIterable(table_traits::MakeKeyIterator(begin()), table_traits::MakeKeyIterator(end())); }
    auto Keys() const { return MakeIterable(table_traits::MakeKeyIterator(cbegin()), table_traits::MakeKeyIterator(cend())); }

    auto Values() { return MakeIterable(table_traits::MakeValueIterator(begin()), table_traits::MakeValueIterator(end())); }
    auto Values() const { return MakeIterable(table_traits::MakeValueIterator(cbegin()), table_traits::MakeValueIterator(cend())); }

    void assign(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); }
    void assign(TBasicHashTable&& rvalue);
    template <typename _It>
    typename std::enable_if<Meta::is_iterator<_It>::value>::type
        assign(_It first, _It last) {
        typedef std::iterator_traits<_It> iterator_traits;
        typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
        clear();
        insert_(first, last, iterator_category());
    }

    void append(const TBasicHashTable& other) {
        insert(other.begin(), other.end());
    }
    void append_AssertUnique(const TBasicHashTable& other) {
        reserve_Additional(other.size());
        insert_AssertUnique(other.begin(), other.end());
    }

    iterator find(const key_type& key);
    const_iterator find(const key_type& key) const;

    mapped_reference at(const key_type& key) { return table_traits::Value(*find(key)); }
    mapped_reference_const at(const key_type& key) const { return table_traits::Value(*find(key)); }

    mapped_reference_const operator [](const key_type& key) const { return at(key); }

    mapped_reference operator [](const key_type& key) { return table_traits::Value(*try_emplace(key).first); }
    mapped_reference operator [](key_type&& rkey) { return table_traits::Value(*try_emplace(std::move(rkey)).first); }

    TPair<iterator, bool> insert(const value_type& value);
    TPair<iterator, bool> insert(value_type&& rvalue);

    TPair<iterator, bool> insert(iterator hint, const value_type& value) { UNUSED(hint); return insert(value); }
    TPair<iterator, bool> insert(iterator hint, value_type&& rvalue) { UNUSED(hint); return insert(std::move(rvalue)); }

    bool insert_ReturnIfExists(const value_type& value) { return (not insert(value).second); }
    void insert_AssertUnique(const value_type& value) {
        const TPair<iterator, bool> it = insert(value);
        Assert(it.second);
        UNUSED(it);
    }

    void insert(std::initializer_list<value_type> ilist) { insert(ilist.begin(), ilist.end()); }

    template <typename _It>
    typename std::enable_if< Meta::is_iterator<_It>::value >::type
        insert(_It first, _It last) {
        typedef std::iterator_traits<_It> iterator_traits;
        typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
        insert_(first, last, iterator_category());
    }

    template <typename _It>
    void insert_AssertUnique(_It first, _It last) {
#ifdef WITH_CORE_ASSERT
        for (; first != last; ++first)
            insert_AssertUnique(*first);
#else
        insert(first, last);
#endif
    }

    TPair<iterator, bool> insert_or_assign(const value_type& value);
    TPair<iterator, bool> insert_or_assign(value_type&& rvalue);

    template <typename _MappedLiked>
    TPair<iterator, bool> insert_or_assign(const key_type& key, _MappedLiked&& rvalue) {
        return insert_or_assign(std::move(value_type(key, std::move(rvalue))));
    }
    template <typename _MappedLiked>
    TPair<iterator, bool> insert_or_assign(key_type&& rkey, _MappedLiked&& rvalue) {
        return insert_or_assign(std::move(value_type(std::move(rkey), std::move(rvalue))));
    }

    template <typename... _Args>
    TPair<iterator, bool> emplace(_Args&&... args) {
        value_type tmp(std::forward<_Args>(args)...);
        return insert(std::move(tmp));
    }

    template <typename... _Args>
    bool emplace_ReturnIfExists(_Args&&... args) {
        const TPair<iterator, bool> it = emplace(std::forward<_Args>(args)...);
        return (not it.second);
    }
    template <typename... _Args>
    void emplace_AssertUnique(_Args&&... args) {
        const TPair<iterator, bool> it = emplace(std::forward<_Args>(args)...);
        Assert(it.second);
    }

    template <typename... _Args>
    TPair<iterator, bool> try_emplace(const key_type& key, _Args&&... args);
    template <typename... _Args>
    TPair<iterator, bool> try_emplace(key_type&& rkey, _Args&&... args);

    void erase(const const_iterator& it);

    bool erase(const key_type& key, value_type* pValueIFP);
    bool erase(const key_type& key) { return erase(key, nullptr); }
    void erase(const_iterator first, const_iterator last) {
        for (; first != last; ++first)
            erase(first);
    }

    void erase_AssertExists(const key_type& key) {
        if (not erase(key))
            AssertNotReached();
    }

    value_type erase_ReturnValue(const key_type& key) {
        value_type value;
        if (not erase(key, &value))
            AssertNotReached();
        return std::move(value);
    }

    void clear();
    void clear_ReleaseMemory();

    void rehash(size_type count);
    void reserve(size_type count);
    void reserve_Additional(size_type count) { reserve(size() + count); }
    void shrink_to_fit();

    void swap(TBasicHashTable& other);
    friend void swap(TBasicHashTable& lhs, TBasicHashTable& rhs) { lhs.swap(rhs); }

    allocator_type get_allocator() const { return static_cast<const allocator_type&>(*this); }

    bool CheckInvariants() const;
    bool AliasesToContainer(const_pointer p) const;

    bool operator ==(const TBasicHashTable& other) const;
    bool operator !=(const TBasicHashTable& other) const { return (not operator ==(other)); }

private:
    STATIC_CONST_INTEGRAL(size_type, NoIndex, size_type(-1));
    STATIC_CONST_INTEGRAL(size_type, MaxLoadFactor, 70);
    STATIC_CONST_INTEGRAL(size_type, SlackFactor, ((100 - MaxLoadFactor) * 128) / 100);

    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }
    const allocator_type& allocator_() const { return static_cast<const allocator_type&>(*this); }

    void allocator_copy_(const allocator_type& other, std::true_type);
    void allocator_copy_(const allocator_type& other, std::false_type) { UNUSED(other); }

    void allocator_move_(allocator_type&& rvalue, std::true_type);
    void allocator_move_(allocator_type&& rvalue, std::false_type) { UNUSED(rvalue); }

    void assign_rvalue_(TBasicHashTable&& rvalue, std::true_type);
    void assign_rvalue_(TBasicHashTable&& rvalue, std::false_type);

    template <typename _It>
    void insert_(_It first, _It last, std::forward_iterator_tag);
    template <typename _It>
    void insert_(_It first, _It last, std::bidirectional_iterator_tag);
    template <typename _It>
    void insert_(_It first, _It last, std::random_access_iterator_tag);

    void swap_(TBasicHashTable& other, std::true_type);
    void swap_(TBasicHashTable& other, std::false_type);

    size_type GrowIFN_ReturnNewCapacity_(size_type atleast) const;
    size_type ShrinkIFN_ReturnNewCapacity_(size_type atleast) const;

    iterator MakeIterator_(size_type bucket) {
        Assert(bucket <= capacity());
        return iterator(*this, reinterpret_cast<public_type*>(_data.GetBuckets() + bucket));
    }
    const_iterator MakeIterator_(size_type bucket) const {
        Assert(bucket <= capacity());
        return const_iterator(*this, reinterpret_cast<const public_type*>(_data.GetBuckets() + bucket));
    }

    size_type FindFilledBucket_(const key_type& key) const;
    size_type FindEmptyBucket_(const key_type& key) const;
    size_type FindOrAllocateBucket_(const key_type& key) const;

    void RelocateRehash_(size_type newCapacity);

    enum class EBucketState : u8 {
        Inactive = 0,   // Never been touched
        Tomb,           // Is inside a search-chain, but is empty
        Filled,         // Is set item
    };

    struct EMPTY_BASES FData_ : hasher, key_equal {
        pointer     StatesAndBuckets;
        u32         CapacityM1;
        u32         Size            : 24;
        mutable u32 MaxProbeDist    : 8;

        FData_() : FData_(hasher{}, key_equal{}) {}
        FData_(hasher&& hash, key_equal&& equal)
            : hasher(std::move(hash))
            , key_equal(std::move(equal))
            , StatesAndBuckets(nullptr)
            , CapacityM1(u32(-1))
            , Size(0)
            , MaxProbeDist(0) {}

#if 0 // scrambling upper bits
        size_type HashKey(const key_type& key) const {
            size_type h = size_type(hasher::operator ()(key));
            while (h > CapacityM1)
                h = (h >> 8) + (h & CapacityM1);
            return h;
        }
#else
        FORCE_INLINE size_type HashKey(const key_type& key) const {
            return size_type(hasher::operator ()(key));
        }
#endif

        FORCE_INLINE bool KeyEqual(const key_type& lhs, const key_type& rhs) const {
            return key_equal::operator ()(lhs, rhs);
        }

        STATIC_CONST_INTEGRAL(size_type, BitsPerState_, 2);
        STATIC_CONST_INTEGRAL(size_type, BitsPerStateLog2_, Meta::template TLog2<BitsPerState_>::value);
        STATIC_CONST_INTEGRAL(size_type, BitsStateMask_, 3);

        FORCE_INLINE static size_type StatesSizeInT(size_type capacity) {
            STATIC_CONST_INTEGRAL(size_type, BitsPerT_, Meta::template TBitCount<value_type>::value);
            STATIC_CONST_INTEGRAL(size_type, BitsPerTM1_, BitsPerT_ - 1);
            STATIC_CONST_INTEGRAL(size_type, BitsPerTLog2_, Meta::template TLog2<BitsPerT_>::value);
            //return ((capacity * BitsPerState_ + BitsPerT_ - 1) / BitsPerT_);
            return (((capacity << BitsPerStateLog2_) + BitsPerTM1_) >> BitsPerTLog2_);
        }

#if     defined(ARCH_64BIT)
        STATIC_ASSERT(sizeof(size_type) == sizeof(u64));
        STATIC_CONST_INTEGRAL(size_type, WordBitMask,  63);
        STATIC_CONST_INTEGRAL(size_type, WordBitShift,  6);
#elif   defined(ARCH_32BIT)
        STATIC_ASSERT(sizeof(size_type) == sizeof(u32));
        STATIC_CONST_INTEGRAL(size_type, WordBitMask,  31);
        STATIC_CONST_INTEGRAL(size_type, WordBitShift,  5);
#else
#   error "unsupported architecture !"
#endif

        EBucketState GetState(size_type index) const;
        void SetState(size_type index, EBucketState state);

        FORCE_INLINE pointer GetBuckets() { return (StatesAndBuckets + StatesSizeInT(CapacityM1 + 1)); }
        FORCE_INLINE const_pointer GetBuckets() const { return const_cast<FData_*>(this)->GetBuckets(); }

        size_type NextBucket(size_type bucket) const;

        void Swap(FData_& other);
    };

    FData_ _data;
};
//----------------------------------------------------------------------------
template <typename _Traits, typename _Hasher, typename _EqualTo, typename _Allocator>
hash_t hash_value(const TBasicHashTable<_Traits, _Hasher, _EqualTo, _Allocator>& hashTable) {
    return hash_range(hashTable.begin(), hashTable.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/HashTable-inl.h"
