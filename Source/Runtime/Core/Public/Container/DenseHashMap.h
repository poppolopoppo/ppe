#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Container/CompressedPair.h"
#include "Container/Hash.h"
#include "Container/HashHelpers.h"
#include "Container/Pair.h"
#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"
#include "Maths/PrimeNumbers.h"
#include "Meta/Iterator.h"

#define DENSEHASHMAP(_DOMAIN, _KEY, _VALUE) \
    ::PPE::TDenseHashMap<_KEY, _VALUE, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, ALLOCATOR(_DOMAIN) >

#define DENSEHASHMAP_MEMOIZE(_DOMAIN, _KEY, _VALUE) \
    DENSEHASHMAP(_DOMAIN, ::PPE::THashMemoizer<_KEY>, _VALUE)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*
** TDenseHashMap<> is similar to std::unordered_map<>,
** except that items are stored packed in a contiguous array and returned pair of references.
*/
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//---------------------------------------------------------------------------
// Store
//---------------------------------------------------------------------------
template <typename _Key, typename _Value>
struct TDenseHashMapNode final {
    using value_type = TPair<_Key, _Value>;

    TDenseHashMapNode(const TDenseHashMapNode&) = default;
    TDenseHashMapNode& operator =(const TDenseHashMapNode&) = default;

    TDenseHashMapNode(TDenseHashMapNode&&) = default;
    TDenseHashMapNode& operator =(TDenseHashMapNode&&) = default;

    template <typename... _Args>
    TDenseHashMapNode(const size_t pos, _Args&&... args) NOEXCEPT
        :   Element(std::forward<_Args>(args)...)
        , Next(pos)
    {}

    value_type Element;
    size_t Next;
};
//---------------------------------------------------------------------------
// Iterate through all elements of a TDenseHashMap<>
//---------------------------------------------------------------------------
template <typename _It>
class TDenseHashMapIterator final {
    template <typename>
    friend class TDenseHashMapIterator;

    using first_type = decltype(std::as_const(std::declval<_It>()->Element.first));
    using second_type = decltype(std::declval<_It>()->Element.second);

public:
    using value_type = TPair<first_type, second_type>; // pair of references, not pair of values!
    using pointer = TInputIteratorPointer<value_type>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;

    CONSTEXPR TDenseHashMapIterator() NOEXCEPT
        : _it{}
    {}

    CONSTEXPR TDenseHashMapIterator(const _It iter) NOEXCEPT
        : _it{ iter }
    {}

    template <typename _Other, typename = Meta::TEnableIf<not std::is_same_v<_It, _Other>&& std::is_constructible_v<_It, _Other>> >
    CONSTEXPR TDenseHashMapIterator(const TDenseHashMapIterator<_Other>& other) NOEXCEPT
        : _it{ other._it }
    {}

    CONSTEXPR TDenseHashMapIterator& operator++() NOEXCEPT {
        return ++_it, * this;
    }

    CONSTEXPR TDenseHashMapIterator operator++(int) NOEXCEPT {
        TDenseHashMapIterator orig = *this;
        return ++(*this), orig;
    }

    CONSTEXPR TDenseHashMapIterator& operator--() NOEXCEPT {
        return --_it, * this;
    }

    CONSTEXPR TDenseHashMapIterator operator--(int) NOEXCEPT {
        TDenseHashMapIterator orig = *this;
        return operator--(), orig;
    }

    CONSTEXPR TDenseHashMapIterator& operator+=(const difference_type value) NOEXCEPT {
        _it += value;
        return *this;
    }

    CONSTEXPR TDenseHashMapIterator operator+(const difference_type value) const NOEXCEPT {
        TDenseHashMapIterator copy = *this;
        return (copy += value);
    }

    CONSTEXPR TDenseHashMapIterator& operator-=(const difference_type value) NOEXCEPT {
        return (*this += -value);
    }

    CONSTEXPR TDenseHashMapIterator operator-(const difference_type value) const NOEXCEPT {
        return (*this + -value);
    }

    NODISCARD CONSTEXPR reference operator[](const difference_type value) const NOEXCEPT {
        const auto& element = _it[value].Element;
        return { element.first, element.second };
    }

    NODISCARD CONSTEXPR pointer operator->() const NOEXCEPT {
        return operator*();
    }

    NODISCARD CONSTEXPR reference operator*() const NOEXCEPT {
        return operator[](0);
    }

    template <typename _Lhs, typename _Rhs>
    friend CONSTEXPR std::ptrdiff_t operator-(const TDenseHashMapIterator<_Lhs>&, const TDenseHashMapIterator<_Rhs>&) NOEXCEPT;

    template <typename _Lhs, typename _Rhs>
    friend CONSTEXPR bool operator==(const TDenseHashMapIterator<_Lhs>&, const TDenseHashMapIterator<_Rhs>&) NOEXCEPT;

    template <typename _Lhs, typename _Rhs>
    friend CONSTEXPR bool operator<(const TDenseHashMapIterator<_Lhs>&, const TDenseHashMapIterator<_Rhs>&) NOEXCEPT;

private:
    _It _it;
};
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR std::ptrdiff_t operator-(const TDenseHashMapIterator<_Lhs>& lhs, const TDenseHashMapIterator<_Rhs>& rhs) NOEXCEPT {
    return (lhs._it - rhs._it);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator==(const TDenseHashMapIterator<_Lhs>& lhs, const TDenseHashMapIterator<_Rhs>& rhs) NOEXCEPT {
    return (lhs._it == rhs._it);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator!=(const TDenseHashMapIterator<_Lhs>& lhs, const TDenseHashMapIterator<_Rhs>& rhs) NOEXCEPT {
    return not (lhs == rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator<(const TDenseHashMapIterator<_Lhs>& lhs, const TDenseHashMapIterator<_Rhs>& rhs) NOEXCEPT {
    return (lhs._it < rhs._it);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator>(const TDenseHashMapIterator<_Lhs>& lhs, const TDenseHashMapIterator<_Rhs>& rhs) NOEXCEPT {
    return (rhs < lhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator<=(const TDenseHashMapIterator<_Lhs>& lhs, const TDenseHashMapIterator<_Rhs>& rhs) NOEXCEPT {
    return not (lhs > rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator>=(const TDenseHashMapIterator<_Lhs>& lhs, const TDenseHashMapIterator<_Rhs>& rhs) NOEXCEPT {
    return not (lhs < rhs);
}
//----------------------------------------------------------------------------
// Iterate only through the elements stored in a specific bucket
//----------------------------------------------------------------------------
template <typename _It>
class TDenseHashMapLocalIterator final {
    template <typename>
    friend class TDenseHashMapLocalIterator;

    using first_type = decltype(std::as_const(std::declval<_It>()->Element.first));
    using second_type = decltype(std::declval<_It>()->Element.second);

public:
    using value_type = TPair<first_type, second_type>; // pair of references, not pair of values!
    using pointer = TInputIteratorPointer<value_type>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;

    CONSTEXPR TDenseHashMapLocalIterator() NOEXCEPT
        : _it{}
        , _offset{}
    {}

    CONSTEXPR TDenseHashMapLocalIterator(_It iter, size_t pos) NOEXCEPT
        : _it(iter)
        , _offset(pos)
    {}

    template <typename _Other, typename = Meta::TEnableIf<not std::is_same_v<_It, _Other>&& std::is_constructible_v<_It, _Other>> >
    CONSTEXPR TDenseHashMapLocalIterator(const TDenseHashMapLocalIterator<_Other>& other) NOEXCEPT
        : _it{ other._it }
        , _offset{ other._offset }
    {}

    CONSTEXPR TDenseHashMapLocalIterator& operator++() NOEXCEPT {
        return _offset = _it[_offset].Next, *this;
    }

    CONSTEXPR TDenseHashMapLocalIterator operator++(int) NOEXCEPT {
        TDenseHashMapLocalIterator orig = *this;
        return ++(*this), orig;
    }

    NODISCARD CONSTEXPR pointer operator->() const NOEXCEPT {
        return operator*();
    }

    NODISCARD CONSTEXPR reference operator*() const NOEXCEPT {
        const auto& element = _it[_offset].Element;
        return { element.first, element.second };
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
NODISCARD constexpr bool operator==(const TDenseHashMapLocalIterator<_Lhs>& lhs, const TDenseHashMapLocalIterator<_Rhs>& rhs) NOEXCEPT {
    return (lhs.index() == rhs.index());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD constexpr bool operator!=(const TDenseHashMapLocalIterator<_Lhs>& lhs, const TDenseHashMapLocalIterator<_Rhs>& rhs) NOEXCEPT {
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
    typename _Value,
    typename _Hasher = Meta::THash<_Key>,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Allocator = ALLOCATOR(Container)
>
class TDenseHashMap {
    static CONSTEXPR const float gDefaultThreshold = 0.875f;
    static CONSTEXPR const size_t gMininumCapacity = 7;

    using node_type = details::TDenseHashMapNode<_Key, _Value>;
    using sparse_container_type = TVector<size_t, _Allocator>;
    using packed_container_type = TVector<node_type, _Allocator>;

public:
    using allocator_type = _Allocator;
    using key_type = _Key;
    using mapped_type = _Value;
    using value_type = TPair<const _Key, _Value>;
    using size_type = size_t;
    using hasher = _Hasher;
    using key_equal = _EqualTo;
    using iterator = details::TDenseHashMapIterator<typename packed_container_type::iterator>;
    using const_iterator = details::TDenseHashMapIterator<typename packed_container_type::const_iterator>;
    using local_iterator = details::TDenseHashMapLocalIterator<typename packed_container_type::iterator>;
    using const_local_iterator = details::TDenseHashMapLocalIterator<typename packed_container_type::const_iterator>;

    TDenseHashMap()
        : TDenseHashMap(gMininumCapacity)
    {}

    explicit TDenseHashMap(const allocator_type& allocator)
        : TDenseHashMap(gMininumCapacity, hasher{}, key_equal{}, allocator)
    {}

    TDenseHashMap(size_type cnt, const allocator_type& allocator)
        : TDenseHashMap(cnt, hasher{}, key_equal{}, allocator)
    {}

    TDenseHashMap(size_type cnt, const hasher& hash, const allocator_type& allocator)
        : TDenseHashMap(cnt, hash, key_equal{}, allocator)
    {}

    explicit TDenseHashMap(size_type cnt, const hasher& hash = hasher{}, const key_equal& equal = key_equal{}, const allocator_type& allocator = allocator_type{})
        : _packed{ allocator, equal }
        , _sparse{ allocator, hash } {
        rehash(cnt);
    }

    TDenseHashMap(const TDenseHashMap& other, const allocator_type& allocator)
        : _packed{ std::piecewise_construct, std::forward_as_tuple(other._packed.first(), allocator), std::forward_as_tuple(other._packed.second()) }
        , _sparse{ std::piecewise_construct, std::forward_as_tuple(other._sparse.first(), allocator), std::forward_as_tuple(other._sparse.second()) }
        , _threshold(other._threshold)
    {}

    TDenseHashMap(const TDenseHashMap&) = default;
    TDenseHashMap& operator =(const TDenseHashMap&) = default;

    TDenseHashMap(TDenseHashMap&&) NOEXCEPT_IF(std::is_nothrow_move_constructible_v<TCompressedPair<sparse_container_type, hasher>>&& std::is_nothrow_move_constructible_v<TCompressedPair<packed_container_type, key_equal>>) = default;
    TDenseHashMap& operator =(TDenseHashMap&&) NOEXCEPT_IF(std::is_nothrow_move_constructible_v<TCompressedPair<sparse_container_type, hasher>>&& std::is_nothrow_move_constructible_v<TCompressedPair<packed_container_type, key_equal>>) = default;

    ~TDenseHashMap() NOEXCEPT = default;

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

    void clear() NOEXCEPT {
        _packed.first().clear();
        _sparse.first().clear();

        rehash(0u);
    }

    void clear_ReleaseMemory() NOEXCEPT {
        _packed.first().clear_ReleaseMemory();
        _sparse.first().clear_ReleaseMemory();

        rehash(0u);
    }

    TPair<iterator, bool> insert(const value_type& value) {
        return Insert_KeepOldIFN_(value.first, value.second);
    }

    TPair<iterator, bool> insert(value_type&& rvalue) {
        return Insert_KeepOldIFN_(std::move(rvalue.first), std::move(rvalue.second));
    }

    template <typename _Arg>
    Meta::TEnableIf<
        std::is_constructible_v<value_type, _Arg&&>,
        TPair<iterator, bool>> insert(_Arg&& arg) {
        return Insert_KeepOldIFN_(std::forward<_Arg>(arg).first, std::forward<_Arg>(arg).second);
    }

    template <typename _It>
    Meta::TEnableIf<Meta::is_iterator_v<_It>> insert(_It first, _It last) {
        IF_CONSTEXPR(Meta::is_random_access_iterator_v<_It>) {
            reserve(checked_cast<size_type>(size() + std::distance(first, last)));
        }

        for (; first != last; ++first)
            insert(*first);
    }

    template <typename _Arg>
    Meta::TEnableIf<
        std::is_constructible_v<mapped_type, _Arg&&>,
        TPair<iterator, bool>> insert_or_assign(const key_type& key, _Arg&& value) {
        return Insert_Overwrite_(key, std::forward<_Arg>(value));
    }

    template <typename _Arg>
    Meta::TEnableIf<
        std::is_constructible_v<mapped_type, _Arg&&>,
        TPair<iterator, bool>> insert_or_assign(key_type&& rkey, _Arg&& value) {
        return Insert_Overwrite_(std::move(rkey), std::forward<_Arg>(value));
    }

    template <typename... _Args>
    TPair<iterator, bool> emplace(MAYBE_UNUSED _Args&&... args) {
        IF_CONSTEXPR(sizeof...(_Args) == 0u) {
            return Insert_KeepOldIFN_(key_type{});
        }
        else IF_CONSTEXPR(sizeof...(_Args) == 1u) {
            return Insert_KeepOldIFN_(std::forward<_Args>(args).first..., std::forward<_Args>(args).second...);
        }
        else IF_CONSTEXPR(sizeof...(_Args) == 2u) {
            return Insert_KeepOldIFN_(std::forward<_Args>(args)...);
        }
        else {
            node_type& node = _packed.first().emplace_back(_packed.first().size(), std::forward<_Args>(args)...);
            const size_type bucket = BucketIndex_(node.second);

            if (const iterator it = ConstrainedFind_(node.Element.first, bucket); it != end()) {
                _packed.first().pop_back();
                return MakePair(it, false);
            }

            std::swap(node.Next, _sparse.first()[bucket]);
            RehashIfRequired_();

            return MakePair(--end(), true);
        }
    }

    template <typename... _Args>
    Meta::TEnableIf<
        std::is_constructible_v<mapped_type, _Args&&...>,
        TPair<iterator, bool>> try_emplace(const key_type& key, _Args&&... args) {
        return Insert_KeepOldIFN_(key, std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    Meta::TEnableIf<
        std::is_constructible_v<mapped_type, _Args&&...>,
        TPair<iterator, bool>> try_emplace(key_type&& rkey, _Args&&... args) {
        return Insert_KeepOldIFN_(std::move(rkey), std::forward<_Args>(args)...);
    }

    iterator erase(const_iterator pos) {
        const auto diff = std::distance(cbegin(), pos);
        erase(pos->first);
        return (begin() + diff);
    }

    iterator erase(const_iterator first, const_iterator last) {
        const auto dist = std::distance(cbegin(), first);

        for (auto from = last - cbegin(); from != dist; --from)
            erase(_packed.first()[from - 1u].Element.first);

        return (begin() + dist);
    }

    size_type erase(const key_type& key) {
        for (size_type* curr = &_sparse.first()[BucketIndex_(key)];
            *curr != (std::numeric_limits<size_type>::max)();
            curr = &_packed.first()[*curr].Next) {
            if (_packed.second()(_packed.first()[*curr].Element.first, key)) {
                const size_type index = *curr;
                *curr = _packed.first()[*curr].Next;
                MoveAndPop_(index);
                return 1u;
            }
        }

        return 0u;
    }

    NODISCARD mapped_type& at(const key_type& key) {
        const iterator it = find(key);
        Assert(it != end());
        return it->second;
    }
    NODISCARD const mapped_type& at(const key_type& key) const {
        const const_iterator it = find(key);
        Assert(it != cend());
        return it->second;
    }

    NODISCARD mapped_type& operator [](const key_type& key) {
        return Insert_KeepOldIFN_(key).first->second;
    }
    NODISCARD mapped_type& operator [](key_type&& rkey) {
        return Insert_KeepOldIFN_(std::move(rkey)).first->second;
    }
    NODISCARD const mapped_type& operator [](const key_type& key) const {
        return at(key);
    }

    void swap(TDenseHashMap& other) NOEXCEPT {
        using std::swap;
        swap(_packed, other._packed);
        swap(_sparse, other._sparse);
        swap(_threshold, other._threshold);
    }

    inline friend void swap(TDenseHashMap& lhs, TDenseHashMap& rhs) {
        lhs.swap(rhs);
    }

    NODISCARD bool contains(const value_type& key) const {
        return (find(key) != cend());
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
                const size_type bucket = BucketIndex_(_packed.first()[pos].Element.first);
                _packed.first()[pos].Next = std::exchange(_sparse.first()[bucket], pos);
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

    NODISCARD decltype(auto) Keys() const NOEXCEPT {
        return MakeIterable(MakeKeyIterator(cbegin()), MakeKeyIterator(cend()));
    }

    NODISCARD decltype(auto) Values() NOEXCEPT {
        return MakeIterable(MakeValueIterator(begin()), MakeValueIterator(end()));
    }

    NODISCARD decltype(auto) Values() const NOEXCEPT {
        return MakeIterable(MakeValueIterator(cbegin()), MakeValueIterator(cend()));
    }

private:
    NODISCARD size_t BucketCount_() const NOEXCEPT {
        return _sparse.first().size();
    }

    template <typename _Other>
    NODISCARD size_type BucketIndex_(const _Other& key) const NOEXCEPT {
        return (static_cast<size_type>(_sparse.second()(key)) % BucketCount_());
    }

    template <typename _Other>
    NODISCARD iterator ConstrainedFind_(const _Other& key, size_type bucket) {
        forrange(it, begin(bucket), end(bucket)) {
            if (_packed.second()(it->first, key))
                return (begin() + static_cast<typename iterator::difference_type>(it.index()));
        }
        return end();
    }

    template <typename _Other>
    NODISCARD const_iterator ConstrainedFind_(const _Other& key, size_type bucket) const {
        forrange(it, cbegin(bucket), cend(bucket)) {
            if (_packed.second()(it->first, key))
                return (cbegin() + static_cast<typename iterator::difference_type>(it.index()));
        }
        return cend();
    }

    template <typename _Other, typename... _Args>
    NODISCARD TPair<iterator, bool> Insert_KeepOldIFN_(_Other&& key, _Args&&... args) {
        const size_type bucket = BucketIndex_(key);

        if (const iterator it = ConstrainedFind_(key, bucket); it != end())
            return MakePair(it, false);

        _packed.first().emplace_back(_sparse.first()[bucket],
            std::piecewise_construct,
            std::forward_as_tuple(std::forward<_Other>(key)),
            std::forward_as_tuple(std::forward<_Args>(args)...));
        _sparse.first()[bucket] = (_packed.first().size() - 1u);

        RehashIfRequired_();

        return MakePair(--end(), true);
    }

    template <typename _Other, typename _Arg>
    NODISCARD TPair<iterator, bool> Insert_Overwrite_(_Other&& key, _Arg&& value) {
        const size_type bucket = BucketIndex_(key);

        if (const iterator it = ConstrainedFind_(key, bucket); it != end()) {
            it->second = std::forward<_Arg>(value);
            return MakePair(it, false);
        }

        _packed.first().emplace_back(_sparse.first()[bucket], std::forward<_Other>(key), std::forward<_Arg>(value));
        _sparse.first()[bucket] = (_packed.first().size() - 1u);

        RehashIfRequired_();

        return MakePair(--end(), true);
    }

    void MoveAndPop_(const size_type pos) {
        Assert_NoAssume(not empty());

        if (const size_type last = (size() - 1u); pos != last) {
            const size_type bucket = BucketIndex_(_packed.first().back().Element.first);
            size_type* curr = &_sparse.first()[bucket];
            _packed.first()[pos] = std::move(_packed.first().back());

            for (; *curr != last; curr = &_packed.first()[*curr].Next);
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
template <typename _Char, typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TDenseHashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashMap) {
    oss << STRING_LITERAL(_Char, "{ ");
    for (const auto& it : hashMap)
        oss << STRING_LITERAL(_Char, '(') << it.first << STRING_LITERAL(_Char, ", ") << it.second << STRING_LITERAL(_Char, "), ");
    return oss << STRING_LITERAL(_Char, '}');
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}; //!namespace PPE
