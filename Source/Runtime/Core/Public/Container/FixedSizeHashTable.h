#pragma once

#include "Core_fwd.h"

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
    using public_type = Meta::TAddConst<value_type>;

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
    using public_type = TPair<Meta::TAddConst<_Key>, _Value>;

    static CONSTEXPR key_type& Key(value_type& v) { return v.first; }
    static CONSTEXPR const key_type& Key(const value_type& v) { return v.first; }

    static CONSTEXPR value_type EmptyValue() {
        STATIC_ASSERT(Meta::is_pod_v<_Key>);
        STATIC_ASSERT(Meta::has_trivial_destructor<_Value>::value);
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
    STATIC_ASSERT(Meta::has_trivial_destructor<value_type>::value); // #TODO support for non POD ?

    using hasher = typename traits_type::hasher;
    using empty_key = typename traits_type::empty_key;
    using key_equal = typename traits_type::key_equal;

    typedef Meta::TAddReference<value_type> reference;
    typedef Meta::TAddReference<const value_type> const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <bool _Const>
    using TPublicType = Meta::TAddConstIFN<typename traits_type::public_type, _Const>;

    template <bool _Const>
    class TIterator : public Meta::TIterator<TPublicType<_Const>> {
    public:
        using parent_type = Meta::TIterator<TPublicType<_Const>>;
        using owner_type = Meta::TAddConstIFN<TFixedSizeHashTable, _Const>;

        using typename parent_type::iterator_category;
        using typename parent_type::difference_type;
        using typename parent_type::value_type;
        using typename parent_type::pointer;
        using typename parent_type::reference;

        TIterator() = default;

        TIterator(const TIterator&) = default;
        TIterator& operator =(const TIterator&) = default;

        template <bool _OtherConst>
        TIterator(const TIterator<_OtherConst>& other) NOEXCEPT : TIterator(other._owner, other._bucket) {}
        template <bool _OtherConst>
        TIterator& operator =(const TIterator<_OtherConst>& other) NOEXCEPT { return operator =(TIterator{ other._owner, other._bucket }); }

        TIterator& operator ++() { return Advance_(); }
        TIterator operator ++(int) { const TIterator tmp(*this); Advance_(); return tmp; }

        reference operator *() const {
            Assert(_bucket < _Capacity);
            return reinterpret_cast<reference>(_owner->_values[_bucket]);
        }
        pointer operator ->() const { return std::addressof(operator *()); }

        template <bool _OtherConst>
        friend bool operator ==(const TIterator& lhs, const TIterator<_OtherConst>& rhs) NOEXCEPT {
            return (lhs._owner == rhs._owner && lhs._bucket == rhs._bucket);
        }
        template <bool _OtherConst>
        friend bool operator !=(const TIterator& lhs, const TIterator<_OtherConst>& rhs) NOEXCEPT {
            return (not operator ==(lhs, rhs));
        }

        void swap(TIterator& lhs, TIterator& rhs) NOEXCEPT {
            std::swap(lhs._owner, rhs._owner);
            std::swap(lhs._bucket, rhs._bucket);
        }

    private:
        friend class TFixedSizeHashTable;

        TIterator(owner_type& owner, size_t bucket) NOEXCEPT
        :   _owner(&owner)
        ,   _bucket(bucket) {
            Assert(_bucket <= _Capacity);
        }

        TIterator& Advance_(size_t amount = 1) NOEXCEPT {
            Assert(_bucket < _Capacity);
            for (_bucket += amount;
                _bucket < _Capacity &&
                traits_type::Key(_owner->_values[_bucket]) == empty_key::value;
                ++_bucket) NOOP();

            return (*this);
        }

        TPtrRef<owner_type> _owner{ nullptr };
        size_t _bucket{ 0 };
    };

    typedef TIterator<false> iterator;
    typedef TIterator<true> const_iterator;

    TFixedSizeHashTable() NOEXCEPT;
    ~TFixedSizeHashTable() NOEXCEPT;

    FORCE_INLINE TFixedSizeHashTable(const TFixedSizeHashTable& other) NOEXCEPT : TFixedSizeHashTable() { operator =(other); }
    TFixedSizeHashTable& operator =(const TFixedSizeHashTable&) NOEXCEPT;

    TFixedSizeHashTable(TFixedSizeHashTable&& rvalue) NOEXCEPT : TFixedSizeHashTable() { operator =(std::move(rvalue)); }
    TFixedSizeHashTable& operator =(TFixedSizeHashTable&&) NOEXCEPT;

    CONSTF bool empty() const NOEXCEPT { return (0 == _size); }
    CONSTF bool full() const NOEXCEPT { return (_Capacity == _size); }
    CONSTF size_t size() const NOEXCEPT { return _size; }

    iterator begin() { return (0 == _size ? end() : iterator(*this, 0).Advance_(0)); }
    iterator end() { return iterator(*this, _Capacity); }

    const_iterator cbegin() const { return (0 == _size ? end() : const_iterator(*this, 0).Advance_(0)); }
    const_iterator cend() const { return const_iterator(*this, _Capacity); }

    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }

    iterator find(const key_type& key) NOEXCEPT;
    const_iterator find(const key_type& key) const NOEXCEPT { return const_cast<TFixedSizeHashTable*>(this)->find(key); }

    TPair<iterator, bool> insert(value_type&& rvalue);

    bool erase(key_type key) { return Remove_ReturnIfExists(key); }
    void erase(const iterator& it) { erase(const_iterator{ it }); }
    void erase(const const_iterator& it);

    void clear();
    void reserve(size_t n) {
        Unused(n);
        Assert_NoAssume(n < _Capacity);
    }

    FORCE_INLINE bool Contains(key_type key) const { return (end() != find(key)); }

    FORCE_INLINE void Add_AssertUnique(value_type&& rvalue) { Verify(Add_KeepExisting(std::move(rvalue))); }
    bool Add_KeepExisting(value_type&& rvalue);
    bool Add_Overwrite(value_type&& rvalue);

    template <typename... _Args>
    void Emplace_AssertUnique(_Args&&... args) { Add_AssertUnique(value_type(std::forward<_Args>(args)...)); }
    template <typename... _Args>
    bool Emplace_KeepExisting(_Args&&... args) { return Add_KeepExisting(value_type(std::forward<_Args>(args)...)); }
    template <typename... _Args>
    bool Emplace_Overwrite(_Args&&... args) { return Add_Overwrite(value_type(std::forward<_Args>(args)...)); }

    FORCE_INLINE void Remove_AssertExists(key_type key) { Verify(Remove_ReturnIfExists(key)); }
    bool Remove_ReturnIfExists(key_type key);

    const_iterator At(size_t bucket) const NOEXCEPT;

    template <size_t _Dim>
    friend bool operator ==(const TFixedSizeHashTable& lhs, const TFixedSizeHashTable<_Traits, _Dim>& rhs) NOEXCEPT {
        if (lhs.size() != rhs.size())
            return false;

        for (const value_type& it : lhs) {
            auto jt = rhs.find(traits_type::Key(it));
            if (jt == rhs.end() || jt->second != it.second)
                return false;
        }

        return true;
    }
    template <size_t _Dim>
    friend bool operator !=(const TFixedSizeHashTable& lhs, const TFixedSizeHashTable<_Traits, _Dim>& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend hash_t hash_value(const TFixedSizeHashTable& table) NOEXCEPT {
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
    Assert_NoAssume(key_equal()(empty_key::value, empty_key::value));
    // need to init table with empty keys
    std::uninitialized_fill(std::begin(_values), std::end(_values), traits_type::EmptyValue());
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
TFixedSizeHashTable<_Traits, _Capacity>::~TFixedSizeHashTable() NOEXCEPT {
    // trivial destructor thanks to PODs
    STATIC_ASSERT(_Capacity > 1); // trivial, but who knows...
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
auto TFixedSizeHashTable<_Traits, _Capacity>::operator =(const TFixedSizeHashTable& other) NOEXCEPT -> TFixedSizeHashTable& {
    // trivial copy/destructor thanks to PODs
    _size = other._size;
    FPlatformMemory::Memcpy(_values, other._values, sizeof(_values));

    return (*this);
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
auto TFixedSizeHashTable<_Traits, _Capacity>::operator =(TFixedSizeHashTable&& rvalue) NOEXCEPT -> TFixedSizeHashTable& {
    // trivial move/destructor thanks to PODs
    _size = rvalue._size;
    FPlatformMemory::Memcpy(_values, rvalue._values, sizeof(_values));
    rvalue.clear();
    return (*this);
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
auto TFixedSizeHashTable<_Traits, _Capacity>::find(const key_type& key) NOEXCEPT -> iterator {
    Assert(empty_key::value != key);

    size_t bucket = InitIndex_(key);

    forrange(i, 0, _Capacity) {
        if (key_equal()(traits_type::Key(_values[bucket]), key))
            return iterator(*this, bucket);

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

    VerifyRelease( ++_size < _Capacity); // this container can't be full!
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
            VerifyRelease( ++_size < _Capacity); // this container can't be full!
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
    return existed;
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

    VerifyRelease( ++_size < _Capacity); // this container can't be full!
    _values[bucket] = std::move(rvalue);

    return MakePair(iterator(*this, bucket), true);
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

    erase(iterator(*this, bucket));

    return true;
}
//----------------------------------------------------------------------------
template <typename _Traits, size_t _Capacity>
void TFixedSizeHashTable<_Traits, _Capacity>::erase(const const_iterator& it) {
    Assert(_size);
    Assert(it._owner.get() == this);
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
auto TFixedSizeHashTable<_Traits, _Capacity>::At(size_t bucket) const NOEXCEPT -> const_iterator {
    bucket = (bucket % _Capacity);
    return (not key_equal()(traits_type::Key(_values[swap]), empty_key::value)
        ? const_iterator{ *this, bucket }
        : end() );
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
class TFixedSizeHashMap : public details::TFixedSizeHashTable<
    details::TFixedSizeHashMapTraits<_Key, _Value, _Hash, _EmptyKey, _EqualTo>,
    _Capacity > {
    using parent_type = details::TFixedSizeHashTable<
        details::TFixedSizeHashMapTraits<_Key, _Value, _Hash, _EmptyKey, _EqualTo>,
        _Capacity >;

public:
    using typename parent_type::key_type;
    using mapped_type = _Value;

    using parent_type::parent_type;
    using parent_type::operator=;

    mapped_type& operator [](const key_type& key) NOEXCEPT { return Get(key); }
    const mapped_type& operator [](const key_type& key) const NOEXCEPT { return Get(key); }

    mapped_type& Get(const key_type& key) NOEXCEPT {
        const auto it = parent_type::find(key);
        AssertRelease(parent_type::end() != it);
        return const_cast<mapped_type&>(it->second);
    }
    const mapped_type& Get(const key_type& key) const NOEXCEPT {
        const auto it = parent_type::find(key);
        AssertRelease(parent_type::end() != it);
        return it->second;
    }

    Meta::TOptionalReference<mapped_type> GetIFP(const key_type& key) NOEXCEPT {
        const auto it = parent_type::find(key);
        return (parent_type::end() != it ? Meta::MakeOptionalRef(const_cast<mapped_type&>(it->second)) : Default);
    }
    Meta::TOptionalReference<const mapped_type> GetIFP(const key_type& key) const NOEXCEPT {
        const auto it = parent_type::find(key);
        return (parent_type::end() != it ? Meta::MakeOptionalRef(it->second) : Default);
    }

    mapped_type GetCopy(const key_type& key, mapped_type&& ifNotFound) const NOEXCEPT {
        const auto it = parent_type::find(key);
        return (parent_type::end() != it ? it->second : std::move(ifNotFound));
    }

    mapped_type& FindOrAdd(const key_type& key) {
        const auto result = parent_type::insert({ key, Default });
        return const_cast<mapped_type&>(result.first->second);
    }

};
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
    const auto it = hashmap.find(key);
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
    Unused(value);
    Remove_AssertExists(hashmap, key);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
