#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Container/Hash.h"
#include "Container/Pair.h"
#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define FLATSET(_DOMAIN, _KEY) \
    ::PPE::TFlatSet<_KEY, ::PPE::Meta::TEqualTo<_KEY>, ::PPE::Meta::TLess<_KEY>, VECTOR(_DOMAIN, _KEY) >
//----------------------------------------------------------------------------
#define FLATSET_INSITU(_DOMAIN, _KEY, _InSituCount) \
    ::PPE::TFlatSet<_KEY, ::PPE::Meta::TEqualTo<_KEY>, ::PPE::Meta::TLess<_KEY>, VECTORINSITU(_DOMAIN, _KEY, _InSituCount) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Less = Meta::TLess<_Key>,
    typename _Vector = TVector<_Key>
>
class TFlatSet {
public:
    typedef _Key value_type;
    typedef _EqualTo value_equal;
    typedef _Less value_less;
    typedef _Vector vector_type;

    typedef typename vector_type::pointer pointer;
    typedef typename vector_type::const_pointer const_pointer;
    typedef typename vector_type::reference reference;
    typedef typename vector_type::const_reference const_reference;

    typedef typename vector_type::size_type size_type;
    typedef typename vector_type::difference_type difference_type;

    typedef typename vector_type::iterator iterator;
    typedef typename vector_type::const_iterator const_iterator;

    typedef typename vector_type::reverse_iterator reverse_iterator;
    typedef typename vector_type::const_reverse_iterator const_reverse_iterator;

    typedef typename std::random_access_iterator_tag iterator_category;

    TFlatSet();
    explicit TFlatSet(size_type capacity);
    ~TFlatSet();

    TFlatSet(TFlatSet&& rvalue) NOEXCEPT;
    TFlatSet& operator =(TFlatSet&& rvalue) NOEXCEPT;

    TFlatSet(const TFlatSet& other);
    TFlatSet& operator =(const TFlatSet& other);

    TFlatSet(std::initializer_list<value_type> values);
    TFlatSet& operator =(std::initializer_list<value_type> values);

    template <typename _It>
    TFlatSet(_It&& begin, _It&& end) { insert(begin, end); }

    bool operator ==(const TFlatSet& other) const { return _vector == other._vector; }
    bool operator !=(const TFlatSet& other) const { return !operator ==(other); }

    const vector_type& Vector() const { return _vector; }

    size_type capacity() const { return _vector.capacity(); }
    size_type size() const { return _vector.size(); }
    bool empty() const { return _vector.empty(); }

    iterator begin() { return _vector.begin(); }
    iterator end() { return _vector.end(); }

    const_iterator begin() const { return _vector.begin(); }
    const_iterator end() const { return _vector.end(); }

    const_iterator cbegin() const { return _vector.begin(); }
    const_iterator cend() const { return _vector.end(); }

    reverse_iterator rbegin() { return _vector.rbegin(); }
    reverse_iterator rend() { return _vector.rend(); }

    const_reverse_iterator rbegin() const { return _vector.rbegin(); }
    const_reverse_iterator rend() const { return _vector.rend(); }

    void reserve(size_type capacity);
    void clear();
    void clear_ReleaseMemory();

    template <typename _KeyLike>
    const _Key& Get(const _KeyLike& key) const;
    template <typename _KeyLike>
    Meta::TOptionalReference<const _Key> GetIFP(const _KeyLike& key) const NOEXCEPT;

    template <typename _KeyLike>
    iterator find(const _KeyLike& key);
    template <typename _KeyLike>
    const_iterator find(const _KeyLike& key) const;

    iterator FindOrAdd(const _Key& key, bool* pAdded);

    template <typename _KeyLike>
    iterator FindAfter(const _KeyLike& key, const iterator& previous);
    template <typename _KeyLike>
    const_iterator FindAfter(const _KeyLike& key, const const_iterator& previous) const;

    bool Insert_ReturnIfExists(_Key&& key);
    void Insert_KeepOldIFN(_Key&& key);
    void Insert_AssertUnique(_Key&& key);
    void Insert_Overwrite(_Key&& key);

    bool Insert_ReturnIfExists(const _Key& key);
    void Insert_KeepOldIFN(const _Key& key);
    void Insert_AssertUnique(const _Key& key);

    bool Erase(const _Key& key);
    void Erase(const const_iterator& it);

    void Remove_AssertExists(const _Key& key);

    template <typename _It>
    void insert(_It&& begin, _It&& end);
    void insert(_Key&& key) { Insert_KeepOldIFN(std::move(key)); }
    TPair<iterator, bool> insert(const _Key& key) {
        bool added = false;
        iterator it = FindOrAdd(key, &added);
        return MakePair(it, added);
    }

    bool erase(const _Key& key) { return Erase(key); }
    void erase(const const_iterator& it) { return Erase(it); }

    size_t HashValue() const { return hash_value(_vector); }

    TMemoryView<value_type> MakeView() { return _vector.MakeView(); }
    TMemoryView<const value_type> MakeView() const { return _vector.MakeView(); }

    friend void swap(TFlatSet& lhs, TFlatSet& rhs) NOEXCEPT {
        swap(lhs._vector, rhs._vector);
    }
private:
    vector_type _vector;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
hash_t hash_value(const TFlatSet<_Key, _EqualTo, _Less, _Vector>& flatSet) NOEXCEPT {
    return flatSet.HashValue();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Key, typename _EqualTo, typename _Less, typename _Vector>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TFlatSet<_Key, _EqualTo, _Less, _Vector>& flatSet) {
    return oss << flatSet.Vector();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    size_t _Capacity,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Less = Meta::TLess<_Key> >
using TFixedSizeFlatSet = TFlatSet<
    _Key, _EqualTo, _Less,
    TVector<_Key, FIXEDSIZE_ALLOCATOR(_Key, _Capacity)> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/FlatSet-inl.h"
