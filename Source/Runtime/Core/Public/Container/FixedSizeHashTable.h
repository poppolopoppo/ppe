#pragma once

#include "Core.h"

#include "Container/Hash.h"
#include "Container/HashHelpers.h"
#include "Container/Pair.h"
#include "HAL/PlatformMemory.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace  details {
//----------------------------------------------------------------------------
// Really minimalist hash table with strong assumptions.
// The user must provide an empty key traits (works from scratch for integral types).
// Uses robin hood hashing with backward shift deletion instead of tombstones.
// Very sensitive to hash function speed, also beware of iteration times which can be slow if the table is large.
// Consider using THashMemoizer<> when the hash isn't trivial.
// Currently accepts only POD types, but could easily be extended if needed.
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Hash = Meta::THash<_Key>,
    typename _EmptyKey = Meta::TEmptyKey<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>
>
struct TFixedSizeHashSetTraits {
    using key_type = _Key;
    using value_type = _Key;
    using hasher = _Hash;
    using empty_key = _EmptyKey;
    using key_equal = _EqualTo;

    static CONSTEXPR key_type& Key(value_type& v) { return v; }
    static CONSTEXPR const key_type& Key(const value_type& v) { return v; }

    static CONSTEXPR value_type EmptyValue() {
        return empty_key::value;
    }
};
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _Hash = Meta::THash<_Key>,
    typename _EmptyKey = Meta::TEmptyKey<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>
>
struct TFixedSizeHashMapTraits {
    using key_type = _Key;
    using value_type = TPair<_Key, _Value>;
    using hasher = _Hash;
    using empty_key = _EmptyKey;
    using key_equal = _EqualTo;

    static CONSTEXPR key_type& Key(value_type& v) { return v.first; }
    static CONSTEXPR const key_type& Key(const value_type& v) { return v.first; }

    static CONSTEXPR value_type EmptyValue() {
        STATIC_ASSERT(Meta::is_pod_v<_Key>);
        STATIC_ASSERT(Meta::is_pod_v<_Value>);
        return MakePair(empty_key::value, Meta::DefaultValue<_Value>());
    }
};
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
class TFixedSizeHashTable {
public:
    STATIC_CONST_INTEGRAL(size_t, Capacity, _Capacity);

    using traits_type = _Traits;
    using key_type = typename traits_type::key_type;
    using value_type = typename traits_type::value_type;
    using hasher = typename traits_type::hasher;
    using empty_key = typename traits_type::empty_key;
    using key_equal = typename traits_type::key_equal;

    typedef Meta::TAddReference<value_type> reference;
    typedef Meta::TAddReference<const value_type> const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    class FIterator : public Meta::TIterator<value_type> {
    public:
        using parent_type = Meta::TIterator<value_type>;

        using typename parent_type::iterator_category;
        using typename parent_type::difference_type;
        using typename parent_type::value_type;
        using typename parent_type::pointer;
        using typename parent_type::reference;

        FIterator(const TFixedSizeHashTable& owner, size_t bucket)
            : _owner(&owner)
            , _bucket(bucket) {
            Assert(_bucket <= _Capacity);
        }

        FIterator(const FIterator&) = default;
        FIterator& operator =(const FIterator&) = default;

        FIterator& operator ++() { return Advance_(); }
        FIterator operator ++(int) { const FIterator tmp(*this); Advance_(); return tmp; }

        const value_type& operator *() const {
            Assert(_bucket < _Capacity);
            return (_owner->_values[_bucket]);
        }
        const value_type* operator ->() const { return &operator *(); }

        friend bool operator ==(const FIterator& lhs, const FIterator& rhs) {
            return (lhs._owner == rhs._owner && lhs._bucket == rhs._bucket);
        }
        friend bool operator !=(const FIterator& lhs, const FIterator& rhs) {
            return (not operator ==(lhs, rhs));
        }

        void swap(FIterator& lhs, FIterator& rhs) {
            std::swap(lhs._owner, rhs._owner);
            std::swap(lhs._bucket, rhs._bucket);
        }

    private:
        const TFixedSizeHashTable* _owner;
        size_t _bucket;

        friend class TFixedSizeHashTable;
        FIterator& Advance_(size_t amount = 1) {
            Assert(_bucket < _Capacity);
            for(_bucket += amount;
                _bucket < _Capacity &&
                traits_type::Key(_owner->_values[_bucket]) == empty_key::value;
                ++_bucket );

            return (*this);
        }
    };

    typedef FIterator iterator;
    typedef FIterator const_iterator;

    TFixedSizeHashTable() NOEXCEPT;
    ~TFixedSizeHashTable() NOEXCEPT;

    FORCE_INLINE TFixedSizeHashTable(const TFixedSizeHashTable& other) : TFixedSizeHashTable() { operator =(other); }
    TFixedSizeHashTable& operator =(const TFixedSizeHashTable&);

    TFixedSizeHashTable(TFixedSizeHashTable&&) = delete;
    TFixedSizeHashTable& operator =(TFixedSizeHashTable&&) = delete;

    bool empty() const NOEXCEPT { return (0 == _size); }
    bool full() const NOEXCEPT { return (_Capacity == _size); }
    size_t size() const NOEXCEPT { return _size; }

    iterator begin() const { return (0 == _size ? end() : FIterator(*this, 0).Advance_(0)); }
    iterator end() const { return FIterator(*this, _Capacity); }

    iterator find(const key_type& key) const;
    TPair<iterator, bool> insert(value_type&& rvalue);
    bool erase(key_type key) { return Remove_ReturnIfExists(key); }
    void erase(const iterator& it);
    void clear();

    void reserve(size_t n) {
        UNUSED(n);
        Assert_NoAssume(n < _Capacity);
    }

    FORCE_INLINE bool Contains(key_type key) const { return (end() != find(key)); }

    FORCE_INLINE void Add_AssertUnique(value_type&& rvalue) { Verify(Add_KeepExisting(std::move(rvalue))); }
    bool Add_KeepExisting(value_type&& rvalue);
    bool Add_Overwrite(value_type&& rvalue);

    template <typename... _Args>
    void Add_AssertUnique(_Args&&... args) { Add_AssertUnique(value_type(std::forward<_Args>(args)...)); }
    template <typename... _Args>
    bool Add_KeepExisting(_Args&&... args) { return Add_KeepExisting(value_type(std::forward<_Args>(args)...)); }
    template <typename... _Args>
    bool Add_Overwrite(_Args&&... args) { return Add_Overwrite(value_type(std::forward<_Args>(args)...)); }

    FORCE_INLINE void Remove_AssertExists(key_type key) { Verify(Remove_ReturnIfExists(key)); }
    bool Remove_ReturnIfExists(key_type key);

    template <size_t _Dim>
    friend bool operator ==(const TFixedSizeHashTable& lhs, const TFixedSizeHashTable<_Traits, _Dim>& rhs) {
        if (lhs.size() != rhs.size())
            return false;

        for (const value_type& it : lhs) {
            auto jt = rhs.find(traits_type::Key(it));
            if (jt == rhs.end() || *jt != it)
                return false;
        }

        return true;
    }
    template <size_t _Dim>
    friend bool operator !=(const TFixedSizeHashTable& lhs, const TFixedSizeHashTable<_Traits, _Dim>& rhs) {
        return (not operator ==(lhs, rhs));
    }

    friend hash_t hash_value(const TFixedSizeHashTable& table) {
        return hash_range(table.begin(), table.end());
    }

private:
    size_t _size;
    value_type _values[_Capacity];

    static size_t InitIndex_(key_type key) NOEXCEPT {
        return (hasher()(key) % _Capacity);
    }
    static FORCE_INLINE CONSTEXPR size_t NextIndexImpl_(size_t b, std::true_type) NOEXCEPT {
        return ((b + 1) % _Capacity);
    }
    static FORCE_INLINE CONSTEXPR size_t NextIndexImpl_(size_t b, std::false_type) NOEXCEPT {
        return (b + 1 < _Capacity ? b + 1 : (b + 1) - _Capacity);
    }
    static CONSTEXPR size_t NextIndex_(size_t b) NOEXCEPT {
        return NextIndexImpl_(b, typename std::bool_constant< Meta::IsPow2(_Capacity) >::type{});
    }
    static FORCE_INLINE CONSTEXPR size_t DistanceIndexImpl_(size_t a, size_t b) NOEXCEPT {
        return (a <= b ? b - a : b + (_Capacity - a));
    }
    static size_t DistanceIndex_(key_type key, size_t b) NOEXCEPT {
        return DistanceIndexImpl_(InitIndex_(key), b);
    }
};
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
TFixedSizeHashTable<_Traits, _Capacity>::TFixedSizeHashTable() NOEXCEPT
:   _size(0) {
    // need to init table with empty keys
    std::uninitialized_fill(std::begin(_values), std::end(_values), traits_type::EmptyValue());
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
TFixedSizeHashTable<_Traits, _Capacity>::~TFixedSizeHashTable() NOEXCEPT {
    // trivial destructor thanks to PODs
    STATIC_ASSERT(_Capacity > 1); // trivial, but who knows...
    STATIC_ASSERT(Meta::is_pod_v<value_type>); // #TODO support for non POD ?
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
auto TFixedSizeHashTable<_Traits, _Capacity>::operator =(const TFixedSizeHashTable& other) -> TFixedSizeHashTable& {
    // trivial destructor thanks to PODs
    _size = other._size;
    FPlatformMemory::Memcpy(_values, other._values, sizeof(_values));

    return (*this);
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
auto TFixedSizeHashTable<_Traits, _Capacity>::find(const key_type& key) const -> iterator {
    Assert(empty_key::value != key);

    size_t bucket = InitIndex_(key);

    forrange(i, 0, _Capacity) {
        if (key_equal()(traits_type::Key(_values[bucket]), key))
            return FIterator(*this, bucket);

        // early out if the probing distance is greater than stored key
        if (key_equal()(traits_type::Key(_values[bucket]), empty_key::value) ||
            i > DistanceIndex_(traits_type::Key(_values[bucket]), bucket) )
                break;

        bucket = NextIndex_(bucket);
    }

    return end();
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
bool TFixedSizeHashTable<_Traits, _Capacity>::Add_KeepExisting(value_type&& rvalue) {
    Assert(_size < _Capacity);
    Assert(empty_key::value != traits_type::Key(rvalue));

    size_t bucket = InitIndex_(traits_type::Key(rvalue));
    size_t dist = 0;
    for (;;) {
        if (key_equal()(traits_type::Key(_values[bucket]), empty_key::value))
            break;

        if (key_equal()(traits_type::Key(_values[bucket]), traits_type::Key(rvalue)))
            return false;

        // keep sorted by probing distance
        const size_t d = DistanceIndex_(traits_type::Key(_values[bucket]), bucket);
        if (dist > d) {
            using std::swap;
            dist = d;
            swap(rvalue, _values[bucket]);
        }

        dist++;
        bucket = NextIndex_(bucket);
    }

    _size++;
    _values[bucket] = std::move(rvalue);

    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
bool TFixedSizeHashTable<_Traits, _Capacity>::Add_Overwrite(value_type&& rvalue) {
    Assert(_size < _Capacity);
    Assert(empty_key::value != traits_type::Key(rvalue));

    bool existed = false;

    size_t bucket = InitIndex_(traits_type::Key(rvalue));
    size_t dist = 0;
    for (;;) {
        if (key_equal()(traits_type::Key(_values[bucket]), empty_key::value)) {
            _size++;
            break;
        }

        if (key_equal()(traits_type::Key(_values[bucket]), traits_type::Key(rvalue))) {
            existed = true;
            break;
        }

        // keep sorted by probing distance
        const size_t d = DistanceIndex_(traits_type::Key(_values[bucket]), bucket);
        if (dist > d) {
            using std::swap;
            dist = d;
            swap(rvalue, _values[bucket]);
        }

        dist++;
        bucket = NextIndex_(bucket);
    }

    _values[bucket] = std::move(rvalue);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
auto TFixedSizeHashTable<_Traits, _Capacity>::insert(value_type&& rvalue) -> TPair<iterator, bool> {
    Assert(_size < _Capacity);
    Assert(empty_key::value != traits_type::Key(rvalue));

    size_t bucket = InitIndex_(traits_type::Key(rvalue));
    size_t dist = 0;
    for (;;) {
        if (key_equal()(traits_type::Key(_values[bucket]), empty_key::value))
            break;

        if (key_equal()(traits_type::Key(_values[bucket]), traits_type::Key(rvalue)))
            return MakePair(iterator(*this, bucket), false);

        // keep sorted by probing distance
        const size_t d = DistanceIndex_(traits_type::Key(_values[bucket]), bucket);
        if (dist > d) {
            using std::swap;
            dist = d;
            swap(rvalue, _values[bucket]);
        }

        dist++;
        bucket = NextIndex_(bucket);
    }

    _size++;
    _values[bucket] = std::move(rvalue);

    return MakePair(FIterator(*this, bucket), true);
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
bool TFixedSizeHashTable<_Traits, _Capacity>::Remove_ReturnIfExists(key_type key) {
    Assert(key != empty_key::value);

    size_t bucket = InitIndex_(key);

    forrange(i, 0, _Capacity) {
        if (key_equal()(traits_type::Key(_values[bucket]), key))
            break;

        if (key_equal()(traits_type::Key(_values[bucket]), empty_key::value) ||
            i > DistanceIndex_(traits_type::Key(_values[bucket]), bucket) )
                return false;

        Assert_NoAssume(i < _Capacity);
        bucket = NextIndex_(bucket);
    }

    erase(FIterator(*this, bucket));

    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
void TFixedSizeHashTable<_Traits, _Capacity>::erase(const iterator& it) {
    Assert(_size);
    Assert(it._owner == this);
    Assert(it._bucket < _Capacity);

    --_size;

    const size_t bucket = it._bucket;

    // backward shift deletion to avoid using tombstones
    forrange(i, 1, _Capacity) {
        const size_t prev = (bucket + i - 1) % _Capacity;
        const size_t swap = (bucket + i) % _Capacity;

        if (key_equal()(traits_type::Key(_values[swap]), empty_key::value) ||
            DistanceIndex_(traits_type::Key(_values[swap]), swap) == 0 ) {
            _values[prev] = traits_type::EmptyValue();
            break;
        }

        _values[prev] = _values[swap];
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
void TFixedSizeHashTable<_Traits, _Capacity>::clear() {
    if (_size) {
        _size = 0;
        std::fill(std::begin(_values), std::end(_values),
            traits_type::EmptyValue() );
    }
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
CONSTEXPR bool is_pod_type(TFixedSizeHashTable<_Traits, _Capacity>*) NOEXCEPT {
    using value_type = typename TFixedSizeHashTable<_Traits, _Capacity>::value_type;
    return Meta::is_pod_v<value_type>;
}
//----------------------------------------------------------------------------
} //!namespace  details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    size_t _Capacity,
    typename _Hash = Meta::THash<_Key>,
    typename _EmptyKey = Meta::TEmptyKey<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>
>
using TFixedSizeHashSet = details::TFixedSizeHashTable<
    details::TFixedSizeHashSetTraits<_Key, _Hash, _EmptyKey, _EqualTo>,
    _Capacity >;
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    size_t _Capacity,
    typename _Hash = Meta::THash<_Key>,
    typename _EmptyKey = Meta::TEmptyKey<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>
>
using TFixedSizeHashMap = details::TFixedSizeHashTable<
    details::TFixedSizeHashMapTraits<_Key, _Value, _Hash, _EmptyKey, _EqualTo>,
    _Capacity >;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
bool TryGetValue(const TFixedSizeHashMap<_Key, _Value, _Capacity, _Hash, _EmptyKey, _EqualTo>& hashmap, const _Key& key, _Value *value) {
    Assert(value);
    const auto it = hashmap.find(key);
    if (hashmap.end() == it) {
        return false;
    }
    else {
        *value = it->second;
        return true;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
bool Insert_ReturnIfExists(TFixedSizeHashMap<_Key, _Value, _Capacity, _Hash, _EmptyKey, _EqualTo>& hashmap, const _Key& key, const _Value& value) {
    return hashmap.Add_KeepExisting(key, value);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
void Insert_AssertUnique(TFixedSizeHashMap<_Key, _Value, _Capacity, _Hash, _EmptyKey, _EqualTo>& hashmap, const _Key& key, const _Value& value) {
    return hashmap.Add_AssertUnique(key, value);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
void Insert_AssertUnique(TFixedSizeHashMap<_Key, _Value, _Capacity, _Hash, _EmptyKey, _EqualTo>& hashmap, _Key&& rkey, _Value&& rvalue) {
    return hashmap.Add_AssertUnique(std::move(rkey), std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
bool Remove_ReturnIfExists(TFixedSizeHashMap<_Key, _Value, _Capacity, _Hash, _EmptyKey, _EqualTo>& hashmap, const _Key& key) {
    return hashmap.erase(key);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
void Remove_AssertExists(TFixedSizeHashMap<_Key, _Value, _Capacity, _Hash, _EmptyKey, _EqualTo>& hashmap, const _Key& key) {
    VerifyRelease(hashmap.erase(key));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
_Value Remove_ReturnValue(TFixedSizeHashMap<_Key, _Value, _Capacity, _Hash, _EmptyKey, _EqualTo>& hashmap, const _Key& key) {
    const auto it = hashmap.find(it);
    AssertRelease_NoAssume(hashmap.end() != it);
    auto value = it->second;
    hashmap.erase(it);
    return value;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
void Remove_AssertExistsAndSameValue(TFixedSizeHashMap<_Key, _Value, _Capacity, _Hash, _EmptyKey, _EqualTo>& hashmap, const _Key& key, const _Value& value) {
#if USE_PPE_ASSERT
    const auto it = hashmap.find(key);
    if (hashmap.end() == it) {
        AssertNotReached();
    }
    else {
        Assert_NoAssume(it->second == value);
        hashmap.erase(it);
    }
#else
    UNUSED(value);
    Remove_AssertExists(hashmap, key);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
