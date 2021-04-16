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
#define FLATMAP(_DOMAIN, _KEY, _VALUE) \
    ::PPE::TFlatMap<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, ::PPE::Meta::TLess<_KEY>, \
        VECTOR(_DOMAIN, ::PPE::TPair<COMMA_PROTECT(_KEY) COMMA COMMA_PROTECT(_VALUE)>) >
//----------------------------------------------------------------------------
#define FLATMAP_INSITU(_DOMAIN, _KEY, _VALUE, _InSituCount) \
    ::PPE::TFlatMap<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, ::PPE::Meta::TLess<_KEY>, \
        VECTORINSITU(_DOMAIN, ::PPE::TPair<COMMA_PROTECT(_KEY) COMMA COMMA_PROTECT(_VALUE)>, _InSituCount) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Less = Meta::TLess<_Key>,
    typename _Vector = TVector<TPair<_Key COMMA _Value>> >
class TFlatMap {
public:
    typedef _Key key_type;
    typedef _Value mapped_type;
    typedef TPair<_Key, _Value> value_type;
    typedef _EqualTo key_equal;
    typedef _Less key_less;
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

    TFlatMap();
    explicit TFlatMap(size_type capacity);
    ~TFlatMap();

    TFlatMap(TFlatMap&& rvalue) NOEXCEPT;
    TFlatMap& operator =(TFlatMap&& rvalue) NOEXCEPT;

    TFlatMap(const TFlatMap& other);
    TFlatMap& operator =(const TFlatMap& other);

    TFlatMap(std::initializer_list<value_type> values);
    TFlatMap& operator =(std::initializer_list<value_type> values);

    template <typename _It>
    TFlatMap(_It&& begin, _It&& end) { insert(begin, end); }

    bool operator ==(const TFlatMap& other) const { return _vector == other._vector; }
    bool operator !=(const TFlatMap& other) const { return !operator ==(other); }

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

    bool Find(const _Key& key, _Value *pvalue) const;

    template <class... _Args>
    bool Emplace_ReturnIfExists(_Key&& key, _Args&&... args);
    template <class... _Args>
    void Emplace_KeepOldIFN(_Key&& key, _Args&&... args);
    template <class... _Args>
    void Emplace_AssertUnique(_Key&& key, _Args&&... args);

    template <class... _Args>
    bool Emplace_ReturnIfExists(const _Key& key, _Args&&... args) { return Emplace_ReturnIfExists(_Key(key), std::forward<_Args>(args)...); }
    template <class... _Args>
    void Emplace_KeepOldIFN(const _Key& key, _Args&&... args) { Emplace_KeepOldIFN(_Key(key), std::forward<_Args>(args)...); }
    template <class... _Args>
    void Emplace_AssertUnique(const _Key& key, _Args&&... args) { Emplace_AssertUnique(_Key(key), std::forward<_Args>(args)...); }

    bool Insert_ReturnIfExists(_Key&& key, _Value&& rvalue);
    void Insert_KeepOldIFN(_Key&& key, _Value&& rvalue);
    void Insert_AssertUnique(_Key&& key, _Value&& rvalue);

    bool Insert_ReturnIfExists(const _Key& key, const _Value& value);
    void Insert_KeepOldIFN(const _Key& key, const _Value& value);
    void Insert_AssertUnique(const _Key& key, const _Value& value);

    _Value& Get(const _Key& key);
    _Value* GetIFP(const _Key& key);
    const _Value* GetIFP(const _Key& key) const { return remove_const(this)->GetIFP(key); }
    bool TryGet(const _Key& key, _Value *value) const;
    const _Value& At(const _Key& key) const;

    _Value& GetOrAdd(const _Key& key) { return FindOrAdd(key, nullptr)->second; }

    bool Erase(const _Key& key);
    void Erase(const const_iterator& it);

    void Remove_AssertExists(const _Key& key, const _Value& valueForDebug);
    void Remove_AssertExists(const _Key& key);

    _Value& operator [](const _Key& key) { return GetOrAdd(key); }
    const _Value& operator [](const _Key& key) const { return At(key); }

    template <typename _It>
    void insert(_It&& begin, _It&& end);

    void shrink_to_fit() { _vector.shrink_to_fit(); }

    size_t HashValue() const NOEXCEPT { return hash_value(_vector); }

    TMemoryView<value_type> MakeView() { return _vector.MakeView(); }
    TMemoryView<const value_type> MakeView() const { return _vector.MakeView(); }

    friend void swap(TFlatMap& lhs, TFlatMap& rhs) NOEXCEPT {
        swap(lhs._vector, rhs._vector);
    }

private:
    struct FKeyEqual_ : key_equal {
        bool operator ()(const value_type& val, const key_type& key) const {
            return key_equal::operator ()(val.first, key);
        }
    };

    struct FKeyLess_ : key_less {
        bool operator ()(const value_type& val, const key_type& key) const {
            return key_less::operator ()(val.first, key);
        }
    };

    vector_type _vector;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
hash_t hash_value(const TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>& flatMap) NOEXCEPT {
    return flatMap.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
FTextWriter& operator <<(FTextWriter& oss, const TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>& flatMap);
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Less, typename _Vector>
FWTextWriter& operator <<(FWTextWriter& oss, const TFlatMap<_Key, _Value, _EqualTo, _Less, _Vector>& flatMap);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/FlatMap-inl.h"
