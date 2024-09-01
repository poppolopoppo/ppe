#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Container/CompressedPair.h"
#include "Container/Hash.h"
#include "Container/Pair.h"
#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"
#include "Maths/PrimeNumbers.h"
#include "Meta/Iterator.h"

#define DENSEHASHSET(_DOMAIN, _KEY) \
    ::PPE::TDenseHashSet<_KEY, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, ALLOCATOR(_DOMAIN) >

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*
** TDenseHashSet<> is similar to std::unordered_set<>,
** except that items are stored packed in a contiguous array with random access.
*/
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//---------------------------------------------------------------------------
// Iterate through all elements of a TDenseHashSet<>
//---------------------------------------------------------------------------
template <typename _It>
class TDenseHashSetIterator final {
    template <typename>
    friend class TDenseHashSetIterator;

    using iterator_traits = Meta::TIteratorTraits<_It>;

public:
    using value_type = typename iterator_traits::value_type::second_type;
    using pointer = Meta::TAddPointer<const value_type>;
    using reference = Meta::TAddReference<const value_type>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    CONSTEXPR TDenseHashSetIterator() NOEXCEPT
        : _it{}
    {}

    CONSTEXPR TDenseHashSetIterator(const _It iter) NOEXCEPT
        : _it{ iter }
    {}

    template <typename _Other, typename = Meta::TEnableIf<not std::is_same_v<_It, _Other>&& std::is_constructible_v<_It, _Other>> >
    CONSTEXPR TDenseHashSetIterator(const TDenseHashSetIterator<_Other>& other) NOEXCEPT
        : _it{ other._it }
    {}

    CONSTEXPR TDenseHashSetIterator& operator++() NOEXCEPT {
        return ++_it, * this;
    }

    CONSTEXPR TDenseHashSetIterator operator++(int) NOEXCEPT {
        TDenseHashSetIterator orig = *this;
        return ++(*this), orig;
    }

    CONSTEXPR TDenseHashSetIterator& operator--() NOEXCEPT {
        return --_it, * this;
    }

    CONSTEXPR TDenseHashSetIterator operator--(int) NOEXCEPT {
        TDenseHashSetIterator orig = *this;
        return operator--(), orig;
    }

    CONSTEXPR TDenseHashSetIterator& operator+=(const difference_type value) NOEXCEPT {
        _it += value;
        return *this;
    }

    CONSTEXPR TDenseHashSetIterator operator+(const difference_type value) const NOEXCEPT {
        TDenseHashSetIterator copy = *this;
        return (copy += value);
    }

    CONSTEXPR TDenseHashSetIterator& operator-=(const difference_type value) NOEXCEPT {
        return (*this += -value);
    }

    CONSTEXPR TDenseHashSetIterator operator-(const difference_type value) const NOEXCEPT {
        return (*this + -value);
    }

    NODISCARD CONSTEXPR reference operator[](const difference_type value) const NOEXCEPT {
        return _it[value].second;
    }

    NODISCARD CONSTEXPR pointer operator->() const NOEXCEPT {
        return std::addressof(operator[](0));
    }

    NODISCARD CONSTEXPR reference operator*() const NOEXCEPT {
        return operator[](0);
    }

    template <typename _Lhs, typename _Rhs>
    friend CONSTEXPR std::ptrdiff_t operator-(const TDenseHashSetIterator<_Lhs>&, const TDenseHashSetIterator<_Rhs>&) NOEXCEPT;

    template <typename _Lhs, typename _Rhs>
    friend CONSTEXPR bool operator==(const TDenseHashSetIterator<_Lhs>&, const TDenseHashSetIterator<_Rhs>&) NOEXCEPT;

    template <typename _Lhs, typename _Rhs>
    friend CONSTEXPR bool operator<(const TDenseHashSetIterator<_Lhs>&, const TDenseHashSetIterator<_Rhs>&) NOEXCEPT;

private:
    _It _it;
};
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR std::ptrdiff_t operator-(const TDenseHashSetIterator<_Lhs>& lhs, const TDenseHashSetIterator<_Rhs>& rhs) NOEXCEPT {
    return (lhs._it - rhs._it);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator==(const TDenseHashSetIterator<_Lhs>& lhs, const TDenseHashSetIterator<_Rhs>& rhs) NOEXCEPT {
    return (lhs._it == rhs._it);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator!=(const TDenseHashSetIterator<_Lhs>& lhs, const TDenseHashSetIterator<_Rhs>& rhs) NOEXCEPT {
    return not (lhs == rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator<(const TDenseHashSetIterator<_Lhs>& lhs, const TDenseHashSetIterator<_Rhs>& rhs) NOEXCEPT {
    return (lhs._it < rhs._it);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator>(const TDenseHashSetIterator<_Lhs>& lhs, const TDenseHashSetIterator<_Rhs>& rhs) NOEXCEPT {
    return (rhs < lhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator<=(const TDenseHashSetIterator<_Lhs>& lhs, const TDenseHashSetIterator<_Rhs>& rhs) NOEXCEPT {
    return not (lhs > rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator>=(const TDenseHashSetIterator<_Lhs>& lhs, const TDenseHashSetIterator<_Rhs>& rhs) NOEXCEPT {
    return not (lhs < rhs);
}
//----------------------------------------------------------------------------
// Iterate only through the elements stored in a specific bucket
//----------------------------------------------------------------------------
template <typename _It>
struct TDenseHashSetLocalIterator final {
    template <typename>
    friend struct TDenseHashSetLocalIterator;

    using iterator_traits = Meta::TIteratorTraits<_It>;

public:
    using value_type = typename iterator_traits::value_type::second_type;
    using pointer = Meta::TAddPointer<const value_type>;
    using reference = Meta::TAddReference<const value_type>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    CONSTEXPR TDenseHashSetLocalIterator() NOEXCEPT
        : _it{}
        , _offset{}
    {}

    CONSTEXPR TDenseHashSetLocalIterator(_It iter, size_t pos) NOEXCEPT
        : _it(iter)
        , _offset(pos)
    {}

    template <typename _Other, typename = Meta::TEnableIf<not std::is_same_v<_It, _Other>&& std::is_constructible_v<_It, _Other>> >
    CONSTEXPR TDenseHashSetLocalIterator(const TDenseHashSetLocalIterator<_Other>& other) NOEXCEPT
        : _it{ other._it }
        , _offset{ other._offset }
    {}

    CONSTEXPR TDenseHashSetLocalIterator& operator++() NOEXCEPT {
        return _offset = _it[_offset].first, *this;
    }

    CONSTEXPR TDenseHashSetLocalIterator operator++(int) NOEXCEPT {
        TDenseHashSetLocalIterator orig = *this;
        return ++(*this), orig;
    }

    NODISCARD CONSTEXPR pointer operator->() const NOEXCEPT {
        return std::addressof(_it[_offset].second);
    }

    NODISCARD CONSTEXPR reference operator*() const NOEXCEPT {
        return *operator->();
    }

    NODISCARD CONSTEXPR std::size_t index() const NOEXCEPT {
        return _offset;
    }

private:
    _It _it;
    size_t _offset;
};
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD constexpr bool operator==(const TDenseHashSetLocalIterator<_Lhs>& lhs, const TDenseHashSetLocalIterator<_Rhs>& rhs) NOEXCEPT {
    return (lhs.index() == rhs.index());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD constexpr bool operator!=(const TDenseHashSetLocalIterator<_Lhs>& lhs, const TDenseHashSetLocalIterator<_Rhs>& rhs) NOEXCEPT {
    return not (lhs == rhs);
}
//---------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Hash set with elements stored in a contiguous vector along a chained list for each bucket
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Hasher = Meta::THash<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container)
>
class TDenseHashSet {
    static CONSTEXPR const float gDefaultThreshold = 0.875f;
    static CONSTEXPR const size_t gMininumCapacity = 7;

    using node_type = TPair<size_t, _Key>;
    using sparse_container_type = TVector<size_t, _Allocator>;
    using packed_container_type = TVector<node_type, _Allocator>;

public:
    using allocator_type = _Allocator;
    using key_type = _Key;
    using value_type = _Key;
    using size_type = size_t;
    using hasher = _Hasher;
    using key_equal = _EqualTo;
    using iterator = details::TDenseHashSetIterator<typename packed_container_type::iterator>;
    using const_iterator = details::TDenseHashSetIterator<typename packed_container_type::const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using reverse_const_iterator = std::reverse_iterator<const_iterator>;
    using local_iterator = details::TDenseHashSetLocalIterator<typename packed_container_type::iterator>;
    using const_local_iterator = details::TDenseHashSetLocalIterator<typename packed_container_type::const_iterator>;

    TDenseHashSet()
        : TDenseHashSet(gMininumCapacity)
    {}

    explicit TDenseHashSet(const allocator_type& allocator)
        : TDenseHashSet(gMininumCapacity, hasher{}, key_equal{}, allocator)
    {}

    TDenseHashSet(size_type capacity, const allocator_type& allocator)
        : TDenseHashSet(capacity, hasher{}, key_equal{}, allocator)
    {}

    TDenseHashSet(size_type capacity, const hasher& hash, const allocator_type& allocator)
        : TDenseHashSet(capacity, hash, key_equal{}, allocator)
    {}

    explicit TDenseHashSet(size_type capacity, const hasher& hash = hasher{}, const key_equal& equal = key_equal{}, const allocator_type& allocator = allocator_type{})
        : _packed{ allocator, equal }
        , _sparse{ allocator, hash } {
        rehash(capacity);
    }

    TDenseHashSet(const TDenseHashSet& other, const allocator_type& allocator)
        : _packed{ std::piecewise_construct, std::forward_as_tuple(other._packed.first(), allocator), std::forward_as_tuple(other._packed.second()) }
        , _sparse{ std::piecewise_construct, std::forward_as_tuple(other._sparse.first(), allocator), std::forward_as_tuple(other._sparse.second()) }
        , _threshold(other._threshold)
    {}

    TDenseHashSet(const TDenseHashSet&) = default;
    TDenseHashSet& operator =(const TDenseHashSet&) = default;

    TDenseHashSet(TDenseHashSet&&) NOEXCEPT_IF(std::is_nothrow_move_constructible_v<TCompressedPair<sparse_container_type, hasher>>&& std::is_nothrow_move_constructible_v<TCompressedPair<packed_container_type, key_equal>>) = default;
    TDenseHashSet& operator =(TDenseHashSet&&) NOEXCEPT_IF(std::is_nothrow_move_constructible_v<TCompressedPair<sparse_container_type, hasher>>&& std::is_nothrow_move_constructible_v<TCompressedPair<packed_container_type, key_equal>>) = default;

    ~TDenseHashSet() NOEXCEPT = default;

    NODISCARD CONSTEXPR bool empty() const NOEXCEPT {
        return _packed.first().empty();
    }

    NODISCARD CONSTEXPR size_type size() const NOEXCEPT {
        return _packed.first().size();
    }

    NODISCARD CONSTEXPR size_type max_size() const NOEXCEPT {
        return _packed.first().max_size();
    }

    NODISCARD CONSTEXPR allocator_type get_allocator() const NOEXCEPT {
        return _sparse.first().get_allocator();
    }

    NODISCARD iterator begin() NOEXCEPT {
        return _packed.first().begin();
    }
    NODISCARD const_iterator begin() const NOEXCEPT {
        return _packed.first().begin();
    }
    NODISCARD const_iterator cbegin() const NOEXCEPT {
        return _packed.first().cbegin();
    }

    NODISCARD iterator end() NOEXCEPT {
        return _packed.first().end();
    }
    NODISCARD const_iterator end() const NOEXCEPT {
        return _packed.first().end();
    }
    NODISCARD const_iterator cend() const NOEXCEPT {
        return _packed.first().cend();
    }

    NODISCARD reverse_iterator rbegin() NOEXCEPT {
        return std::make_reverse_iterator(end());
    }
    NODISCARD reverse_const_iterator rbegin() const NOEXCEPT {
        return std::make_reverse_iterator(end());
    }
    NODISCARD reverse_const_iterator crbegin() const NOEXCEPT {
        return std::make_reverse_iterator(cend());
    }

    NODISCARD reverse_iterator rend() NOEXCEPT {
        return std::make_reverse_iterator(begin());
    }
    NODISCARD reverse_const_iterator rend() const NOEXCEPT {
        return std::make_reverse_iterator(begin());
    }
    NODISCARD reverse_const_iterator crend() const NOEXCEPT {
        return std::make_reverse_iterator(cbegin());
    }

    void clear() NOEXCEPT {
        _packed.first().clear();
        _sparse.first().clear();

        rehash(0u);
    }

    TPair<iterator, bool> insert(const value_type& value) {
        return Insert_KeepOldIFN_(value);
    }

    TPair<iterator, bool> insert(value_type&& rvalue) {
        return Insert_KeepOldIFN_(std::move(rvalue));
    }

    template <typename _It>
    Meta::TEnableIf<Meta::is_iterator_v<_It>> insert(_It first, _It last) {
        IF_CONSTEXPR(Meta::is_random_access_iterator_v<_It>) {
            reserve(checked_cast<size_type>(size() + std::distance(first, last)));
        }

        for (; first != last; ++first)
            insert(*first);
    }

    template <typename... _Args>
    TPair<iterator, bool> emplace(_Args&&... args) {
        IF_CONSTEXPR(((sizeof...(_Args) == 1u) and ... and std::is_same_v<std::decay_t<_Args>, value_type>)) {
            return Insert_KeepOldIFN_(std::forward<_Args>(args)...);
        }
        else {
            node_type& node = _packed.first().emplace_back(std::piecewise_construct,
                std::make_tuple(_packed.first().size()),
                std::forward_as_tuple(std::forward<_Args>(args)...));
            const size_type bucket = BucketIndex_(node.second);

            if (const iterator it = ConstrainedFind_(node.second, bucket); it != end()) {
                _packed.first().pop_back();
                return MakePair(it, false);
            }

            std::swap(node.first, _sparse.first()[bucket]);
            RehashIfRequired_();

            return MakePair(--end(), true);
        }
    }

    iterator erase(const_iterator pos) {
        const auto diff = std::distance(cbegin(), pos);
        erase(*pos);
        return (begin() + diff);
    }

    iterator erase(const_iterator first, const_iterator last) {
        const auto dist = std::distance(cbegin(), first);

        for (auto from = last - cbegin(); from != dist; --from)
            erase(_packed.first()[from - 1u].second);

        return (begin() + dist);
    }

    size_type erase(const value_type& value) {
        for (size_type* curr = &_sparse.first()[BucketIndex_(value)];
            *curr != (std::numeric_limits<size_type>::max)();
            curr = &_packed.first()[*curr].first) {
            if (_packed.second()(_packed.first()[*curr].second, value)) {
                const size_type index = *curr;
                *curr = _packed.first()[*curr].first;
                MoveAndPop_(index);
                return 1u;
            }
        }

        return 0u;
    }

    void swap(TDenseHashSet& other) NOEXCEPT {
        using std::swap;
        swap(_packed, other._packed);
        swap(_sparse, other._sparse);
        swap(_threshold, other._threshold);
    }

    inline friend void swap(TDenseHashSet& lhs, TDenseHashSet& rhs) {
        lhs.swap(rhs);
    }

    NODISCARD bool contains(const value_type& key) const {
        return (find(key) != cend());
        ;
    }

    NODISCARD size_type count(const value_type& key) const {
        return (contains(key) ? 1 : 0);
    }

    NODISCARD iterator find(const value_type& value) {
        return ConstrainedFind_(value, BucketIndex_(value));
    }

    NODISCARD const_iterator find(const value_type& value) const {
        return ConstrainedFind_(value, BucketIndex_(value));
    }

    NODISCARD TPair<iterator, iterator> equal_range(const value_type& value) {
        const iterator it = find(value);
        return { it, it + (it == end() ? 0 : 1) };
    }

    NODISCARD TPair<const_iterator, const_iterator> equal_range(const value_type& value) const {
        const const_iterator it = find(value);
        return { it, it + (it == end() ? 0 : 1) };
    }

    NODISCARD local_iterator begin(const size_type bucket) NOEXCEPT {
        return { _packed.first().begin(), _sparse.first()[bucket] };
    }
    NODISCARD const_local_iterator cbegin(const size_type bucket) const NOEXCEPT {
        return { _packed.first().cbegin(), _sparse.first()[bucket] };
    }
    NODISCARD const_local_iterator begin(const size_type bucket) const NOEXCEPT {
        return cbegin(bucket);
    }

    NODISCARD local_iterator end(const size_type) NOEXCEPT {
        return { _packed.first().begin(), (std::numeric_limits<size_type>::max)() };
    }
    NODISCARD const_local_iterator cend(const size_type) const NOEXCEPT {
        return { _packed.first().cbegin(), (std::numeric_limits<size_type>::max)() };
    }
    NODISCARD const_local_iterator end(const size_type bucket) const NOEXCEPT {
        return cend(bucket);
    }

    NODISCARD size_type bucket(const key_type& key) const NOEXCEPT {
        return BucketIndex_(key);
    }

    NODISCARD size_type bucket_count() const NOEXCEPT {
        return BucketCount_();
    }

    NODISCARD size_type max_bucket_count() const NOEXCEPT {
        return _sparse.first().max_size();
    }

    NODISCARD size_type bucket_size(size_type bucket) const NOEXCEPT {
        return checked_cast<size_type>(std::distance(cbegin(bucket), cend(bucket)));
    }

    NODISCARD float load_factor() const NOEXCEPT {
        return (size() / static_cast<float>(BucketCount_()));
    }

    NODISCARD float max_load_factor() const NOEXCEPT {
        return _threshold;
    }

    void max_load_factor(const float value) {
        Assert(value > 0.f);
        _threshold = value;

        rehash(0u);
    }

    void rehash(size_type cnt) {
        cnt = Max(cnt, gMininumCapacity);
        cnt = Max(cnt, static_cast<size_type>(size() / max_load_factor()));

        const size_type nextBucketCount = static_cast<size_type>(
            FGoodHashTablePrimesU32::ClosestCeil(checked_cast<u32>(cnt)));

        if (nextBucketCount != BucketCount_()) {
            _sparse.first().resize(nextBucketCount);

            std::fill(_sparse.first().begin(), _sparse.first().end(),
                (std::numeric_limits<size_type>::max)());

            for (size_type pos{}, last = size(); pos < last; ++pos) {
                const size_type bucket = BucketIndex_(_packed.first()[pos].second);
                _packed.first()[pos].first = std::exchange(_sparse.first()[bucket], pos);
            }
        }
    }

    void reserve(const size_type cnt) {
        _packed.first().reserve(cnt);
        rehash(static_cast<size_type>(std::ceil(cnt / max_load_factor())));
    }

    NODISCARD hasher hash_function() const NOEXCEPT {
        return _sparse.second();
    }

    NODISCARD key_equal key_eq() const NOEXCEPT {
        return _packed.second();
    }

    NODISCARD TIterable<const_iterator> Keys() const NOEXCEPT {
        return { cbegin(), cend() };
    }

private:
    NODISCARD size_type BucketCount_() const NOEXCEPT {
        return static_cast<size_type>(_sparse.first().size());
    }

    template <typename _Other>
    NODISCARD size_type BucketIndex_(const _Other& value) const NOEXCEPT {
        return (static_cast<size_type>(_sparse.second()(value)) % BucketCount_());
    }

    template <typename _Other>
    NODISCARD iterator ConstrainedFind_(const _Other& value, size_type bucket) {
        forrange(it, begin(bucket), end(bucket)) {
            if (_packed.second()(*it, value))
                return (begin() + static_cast<typename iterator::difference_type>(it.index()));
        }
        return end();
    }

    template <typename _Other>
    NODISCARD const_iterator ConstrainedFind_(const _Other& value, size_type bucket) const {
        forrange(it, cbegin(bucket), cend(bucket)) {
            if (_packed.second()(*it, value))
                return (cbegin() + static_cast<typename iterator::difference_type>(it.index()));
        }
        return cend();
    }

    template <typename _Other>
    NODISCARD TPair<iterator, bool> Insert_KeepOldIFN_(_Other&& value) {
        const size_type bucket = BucketIndex_(value);

        if (const iterator it = ConstrainedFind_(value, bucket); it != end())
            return MakePair(it, false);

        _packed.first().emplace_back(_sparse.first()[bucket], std::forward<_Other>(value));
        _sparse.first()[bucket] = (_packed.first().size() - 1u);

        RehashIfRequired_();

        return MakePair(--end(), true);
    }

    void MoveAndPop_(const size_type pos) {
        Assert_NoAssume(not empty());

        if (const size_type last = (size() - 1u); pos != last) {
            const size_type bucket = BucketIndex_(_packed.first().back().second);
            size_type* curr = &_sparse.first()[bucket];
            _packed.first()[pos] = std::move(_packed.first().back());

            for (; *curr != last; curr = &_packed.first()[*curr].first);
            *curr = pos;
        }

        _packed.first().pop_back();
    }

    void RehashIfRequired_() {
        if (const size_type bucketCount = BucketCount_(); size() > (bucketCount * max_load_factor()))
            rehash(bucketCount * 2u);
    }

    TCompressedPair<packed_container_type, key_equal> _packed;
    TCompressedPair<sparse_container_type, hasher> _sparse;
    float _threshold{ gDefaultThreshold };
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Key, typename _Hash, typename _EqualTo, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TDenseHashSet<_Key, _Hash, _EqualTo, _Allocator>& set) {
    if (set.empty()) {
        return oss << STRING_LITERAL(_Char, "[]");
    }
    else {
        auto it = set.begin();
        oss << STRING_LITERAL(_Char, '[') << *it++;
        for (const auto end = set.end(); it != end; ++it)
            oss << STRING_LITERAL(_Char, ", ") << *it;
        return oss << STRING_LITERAL(_Char, ']');
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}; //!namespace PPE
