#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Hash.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/IO/TextWriter_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define FLAT_SET(_DOMAIN, _KEY) \
    ::Core::TFlatSet<_KEY, ::Core::Meta::TEqualTo<_KEY>, ::Core::Meta::TLess<_KEY>, VECTOR(_DOMAIN, _KEY) >
//----------------------------------------------------------------------------
#define FLAT_SET_THREAD_LOCAL(_DOMAIN, _KEY) \
    ::Core::TFlatSet<_KEY, ::Core::Meta::TEqualTo<_KEY>, ::Core::Meta::TLess<_KEY>, VECTOR_THREAD_LOCAL(_DOMAIN, _KEY) >
//----------------------------------------------------------------------------
#define FLAT_SETINSITU(_DOMAIN, _KEY, _InSituCount) \
    ::Core::TFlatSet<_KEY, ::Core::Meta::TEqualTo<_KEY>, ::Core::Meta::TLess<_KEY>, VECTORINSITU(_DOMAIN, _KEY, _InSituCount) >
//----------------------------------------------------------------------------
#define FLAT_SETINSITU_THREAD_LOCAL(_DOMAIN, _KEY, _InSituCount) \
    ::Core::TFlatSet<_KEY, ::Core::Meta::TEqualTo<_KEY>, ::Core::Meta::TLess<_KEY>, VECTORINSITU_THREAD_LOCAL(_DOMAIN, _KEY, _InSituCount) >
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

    TFlatSet(TFlatSet&& rvalue);
    TFlatSet& operator =(TFlatSet&& rvalue);

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

    reverse_iterator rbegin() { return _vector.rbegin(); }
    reverse_iterator rend() { return _vector.rend(); }

    const_reverse_iterator rbegin() const { return _vector.rbegin(); }
    const_reverse_iterator rend() const { return _vector.rend(); }

    void reserve(size_type capacity);
    void clear();
    void clear_ReleaseMemory();

    iterator Find(const _Key& key);
    iterator FindOrAdd(const _Key& key, bool* pAdded);
    const_iterator Find(const _Key& key) const;

    iterator FindAfter(const _Key& key, const iterator& previous);
    const_iterator FindAfter(const _Key& key, const const_iterator& previous) const;

    bool Insert_ReturnIfExists(_Key&& key);
    void Insert_KeepOldIFN(_Key&& key);
    void Insert_AssertUnique(_Key&& key);

    bool Insert_ReturnIfExists(const _Key& key);
    void Insert_KeepOldIFN(const _Key& key);
    void Insert_AssertUnique(const _Key& key);

    bool Erase(const _Key& key);
    void Erase(const const_iterator& it);

    void Remove_AssertExists(const _Key& key);

    template <typename _It>
    void insert(_It&& begin, _It&& end);
    void insert(_Key&& key) { Insert_KeepOldIFN(std::move(key)); }
    void insert(const _Key& key) { Insert_KeepOldIFN(key); }

    iterator find(const _Key& key) { return Find(key); }
    const_iterator find(const _Key& key) const { return Find(key); }

    bool erase(const _Key& key) { return Erase(key); }
    void erase(const const_iterator& it) { return Erase(it); }

    size_t HashValue() const { return hash_value(_vector); }

    TMemoryView<value_type> MakeView() { return _vector.MakeView(); }
    TMemoryView<const value_type> MakeView() const { return _vector.MakeView(); }

    friend void swap(TFlatSet& lhs, TFlatSet& rhs) {
        swap(lhs._vector, rhs._vector);
    }
private:
    vector_type _vector;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _EqualTo, typename _Less, typename _Vector>
hash_t hash_value(const TFlatSet<_Key, _EqualTo, _Less, _Vector>& flatSet) {
    return flatSet.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Key, typename _EqualTo, typename _Less, typename _Vector>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TFlatSet<_Key, _EqualTo, _Less, _Vector>& flatSet) {
    return oss << flatSet.Vector();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/FlatSet-inl.h"
