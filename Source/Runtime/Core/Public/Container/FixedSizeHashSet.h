#pragma once

#include "Core.h"

#include "Container/Hash.h"
#include "Meta/AlignedStorage.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Really minimalist hash set with strong assumptions.
// The user must provide an empty key traits (works from scratch for integral types).
// Uses robin hood hashing with backward shift deletion instead of tombstones.
// Very sensitive to hash function speed.
// Consider using THashMemoizer<> when the hash isn't trivial.
// Currently accepts only POD types, but could easily be extended if needed.
//----------------------------------------------------------------------------
template <
    typename _Key
,   size_t _Capacity
,   typename _Hash = Meta::THash<_Key>
,   typename _EmptyKey = Meta::TEmptyKey<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
>   class TFixedSizeHashSet {
public:
    typedef _Key value_type;
    typedef _Hash hasher;
    typedef _EmptyKey empty_key;
    typedef _EqualTo key_equal;

    typedef Meta::TAddReference<value_type> reference;
    typedef Meta::TAddReference<const value_type> const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    class FIterator : public Meta::TIterator<_Key> {
    public:
        using parent_type = Meta::TIterator<_Key>;

        using typename parent_type::iterator_category;
        using typename parent_type::difference_type;
        using typename parent_type::value_type;
        using typename parent_type::pointer;
        using typename parent_type::reference;

        FIterator(const TFixedSizeHashSet& owner, size_t bucket)
            : _owner(&owner)
            , _bucket(bucket) {
            Assert(_bucket <= _Capacity);
        }

        FIterator(const FIterator&) = default;
        FIterator& operator =(const FIterator&) = default;

        FIterator& operator ++() { return Advance_(); }
        FIterator operator ++(int) { const FIterator tmp(*this); Advance_(); return tmp; }

        const _Key& operator *() const {
            Assert(_bucket < _Capacity);
            return (_owner->_values[_bucket]);
        }
        const _Key* operator ->() const { return &operator *(); }

        inline friend bool operator ==(const FIterator& lhs, const FIterator& rhs) {
            return (lhs._owner == rhs._owner && lhs._bucket == rhs._bucket);
        }
        inline friend bool operator !=(const FIterator& lhs, const FIterator& rhs) {
            return (not operator ==(lhs, rhs));
        }

        inline void swap(FIterator& lhs, FIterator& rhs) {
            std::swap(lhs._owner, rhs._owner);
            std::swap(lhs._bucket, rhs._bucket);
        }

    private:
        const TFixedSizeHashSet* _owner;
        size_t _bucket;

        friend class TFixedSizeHashSet;
        FIterator& Advance_(size_t amount = 1) {
            Assert(_bucket < _Capacity);
            for(_bucket += amount;
                _bucket < _Capacity &&
                _owner->_values[_bucket] == empty_key::value;
                ++_bucket );

            return (*this);
        }
    };

    typedef FIterator iterator;
    typedef FIterator const_iterator;

    TFixedSizeHashSet() NOEXCEPT;
    ~TFixedSizeHashSet() NOEXCEPT;

    FORCE_INLINE TFixedSizeHashSet(const TFixedSizeHashSet& other) : TFixedSizeHashSet() { operator =(other); }
    TFixedSizeHashSet& operator =(const TFixedSizeHashSet&);

    TFixedSizeHashSet(TFixedSizeHashSet&&) = delete;
    TFixedSizeHashSet& operator =(TFixedSizeHashSet&&) = delete;

    bool empty() const NOEXCEPT { return (0 == _size); }
    bool full() const NOEXCEPT { return (_Capacity == _size); }
    size_t size() const NOEXCEPT { return _size; }

    iterator begin() const { return (0 == _size ? end() : FIterator(*this, 0).Advance_(0)); }
    iterator end() const { return FIterator(*this, _Capacity); }

    FORCE_INLINE bool Contains(_Key key) const { return (end() != Find(key)); }
    iterator Find(_Key key) const;

    FORCE_INLINE void Add_AssertUnique(_Key key) { Verify(Add_KeepExisting(key)); }
    bool Add_KeepExisting(_Key key);

    FORCE_INLINE void Remove_AssertExists(_Key key) { Verify(Remove_ReturnIfExists(key)); }
    bool Remove_ReturnIfExists(_Key key);

    void Erase(const iterator& it);

    void Clear();

private:
    size_t _size;
    _Key _values[_Capacity];

    static size_t InitIndex_(_Key key) NOEXCEPT {
        return (hasher()(key) % _Capacity);
    }
    static FORCE_INLINE CONSTEXPR size_t NextIndexImpl_(size_t b, std::true_type) NOEXCEPT {
        return ((b + 1) % _Capacity);
    }
    static FORCE_INLINE CONSTEXPR size_t NextIndexImpl_(size_t b, std::false_type) NOEXCEPT {
        return (b + 1 < _Capacity ? b + 1 : (b + 1) - _Capacity);
    }
    static CONSTEXPR size_t NextIndex_(size_t b) NOEXCEPT {
        return NextIndexImpl_(b, std::bool_constant< Meta::IsPow2(_Capacity) >::type{});
    }
    static FORCE_INLINE CONSTEXPR size_t DistanceIndexImpl_(size_t a, size_t b) NOEXCEPT {
        return (a <= b ? b - a : b + (_Capacity - a));
    }
    static size_t DistanceIndex_(_Key key, size_t b) NOEXCEPT {
        return DistanceIndexImpl_(InitIndex_(key), b);
    }
};
//----------------------------------------------------------------------------
template <typename _Key, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
TFixedSizeHashSet<_Key, _Capacity, _Hash, _EmptyKey, _EqualTo>::TFixedSizeHashSet() NOEXCEPT
:   _size(0) {
    // need to init table with empty keys
    std::uninitialized_fill(std::begin(_values), std::end(_values), empty_key::value);
}
//----------------------------------------------------------------------------
template <typename _Key, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
TFixedSizeHashSet<_Key, _Capacity, _Hash, _EmptyKey, _EqualTo>::~TFixedSizeHashSet() NOEXCEPT {
    // trivial destructor thanks to PODs
    STATIC_ASSERT(_Capacity > 1); // trivial, but who knows...
    STATIC_ASSERT(Meta::TIsPod_v<_Key>); // #TODO support for non POD ?
}
//----------------------------------------------------------------------------
template <typename _Key, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
auto TFixedSizeHashSet<_Key, _Capacity, _Hash, _EmptyKey, _EqualTo>::operator =(const TFixedSizeHashSet& other) -> TFixedSizeHashSet& {
    // trivial destructor thanks to PODs
    _size = other._size;
    FPlatformMemory::MemcpyLarge(_values, other._values, sizeof(_values));

    return (*this);
}
//----------------------------------------------------------------------------
template <typename _Key, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
auto TFixedSizeHashSet<_Key, _Capacity, _Hash, _EmptyKey, _EqualTo>::Find(_Key key) const -> iterator {
    Assert(empty_key::value != key);

    size_t bucket = InitIndex_(key);

    forrange(i, 0, _Capacity) {
        if (key_equal()(_values[bucket], key))
            return FIterator(*this, bucket);

        // early out if the probing distance is greater than stored key
        if (key_equal()(_values[bucket], empty_key::value) ||
            i > DistanceIndex_(_values[bucket], bucket) )
            break;

        bucket = NextIndex_(bucket);
    }

    return end();
}
//----------------------------------------------------------------------------
template <typename _Key, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
bool TFixedSizeHashSet<_Key, _Capacity, _Hash, _EmptyKey, _EqualTo>::Add_KeepExisting(_Key key) {
    Assert(_size < _Capacity);
    Assert(empty_key::value != key);

    size_t bucket = InitIndex_(key);
    size_t dist = 0;
    for (;;) {
        if (key_equal()(_values[bucket], empty_key::value))
            break;

        if (key_equal()(_values[bucket], key))
            return false;

        // keep sorted by probing distance
        const size_t d = DistanceIndex_(_values[bucket], bucket);
        if (dist > d) {
            using std::swap;
            dist = d;
            swap(key, _values[bucket]);
        }

        dist++;
        bucket = NextIndex_(bucket);
    }

    _size++;
    _values[bucket] = key;

    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
bool TFixedSizeHashSet<_Key, _Capacity, _Hash, _EmptyKey, _EqualTo>::Remove_ReturnIfExists(_Key key) {
    Assert(key != empty_key::value);

    size_t bucket = InitIndex_(key);

    forrange(i, 0, _Capacity) {
        if (key_equal()(_values[bucket], key))
            break;

        if (key_equal()(_values[bucket], empty_key::value) ||
            i > DistanceIndex_(_values[bucket], bucket) )
            return false;

        Assert_NoAssume(i < _Capacity);
        bucket = NextIndex_(bucket);
    }

    Erase(FIterator(*this, bucket));

    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
void TFixedSizeHashSet<_Key, _Capacity, _Hash, _EmptyKey, _EqualTo>::Erase(const iterator& it) {
    Assert(_size);
    Assert(it._owner == this);
    Assert(it._bucket < _Capacity);

    --_size;

    const size_t bucket = it._bucket;

    // backward shift deletion to avoid using tombstones
    forrange(i, 1, _Capacity) {
        const size_t prev = (bucket + i - 1) % _Capacity;
        const size_t swap = (bucket + i) % _Capacity;

        if (key_equal()(_values[swap], empty_key::value) ||
            DistanceIndex_(_values[swap], swap) == 0 ) {
            _values[prev] = empty_key::value;
            break;
        }

        _values[prev] = _values[swap];
    }
}
//----------------------------------------------------------------------------
template <typename _Key, size_t _Capacity, typename _Hash, typename _EmptyKey, typename _EqualTo>
void TFixedSizeHashSet<_Key, _Capacity, _Hash, _EmptyKey, _EqualTo>::Clear() {
    if (_size) {
        _size = 0;
        for (_Key& value : _values)
            value = empty_key::value;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
