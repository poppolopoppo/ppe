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
#define ASSOCIATIVE_VECTOR(_DOMAIN, _KEY, _VALUE) \
    ::PPE::TAssociativeVector<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, VECTOR(_DOMAIN, ::PPE::TPair<COMMA_PROTECT(_KEY) COMMA COMMA_PROTECT(_VALUE)>) >
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTORINSITU(_DOMAIN, _KEY, _VALUE, _InSituCount) \
    ::PPE::TAssociativeVector<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, VECTORINSITU(_DOMAIN, ::PPE::TPair<COMMA_PROTECT(_KEY) COMMA COMMA_PROTECT(_VALUE)>, _InSituCount) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _EqualTo = Meta::TEqualTo<_Key>,
    typename _Vector = TVector<TPair<_Key COMMA _Value>> >
class TAssociativeVector {
public:
    typedef _Key key_type;
    typedef _Value mapped_type;
    typedef TPair<_Key, _Value> value_type;
    typedef _EqualTo key_equal;
    typedef _Vector vector_type;

    typedef typename vector_type::allocator_type allocator_type;

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

    TAssociativeVector() = default;
    ~TAssociativeVector() = default;

    explicit TAssociativeVector(allocator_type&& alloc);
    explicit TAssociativeVector(Meta::FForceInit);
    explicit TAssociativeVector(size_type capacity);

    template <typename _It>
    TAssociativeVector(_It&& begin, _It&& end)
#if USE_PPE_DEBUG
        { insert(begin, end); }
#else
        : _vector(begin, end) {}
#endif

    TAssociativeVector(std::initializer_list<value_type> values) : _vector(values) {}
    TAssociativeVector& operator =(std::initializer_list<value_type> values) {
        _vector.assign(values.begin(), values.end());
        return *this;
    }

    explicit TAssociativeVector(vector_type&& vector) NOEXCEPT;
    TAssociativeVector& operator =(vector_type&& vector) NOEXCEPT;

    explicit TAssociativeVector(const vector_type& vector);
    TAssociativeVector& operator =(const vector_type& vector);

    TAssociativeVector(TAssociativeVector&& rvalue) NOEXCEPT;
    TAssociativeVector& operator =(TAssociativeVector&& rvalue) NOEXCEPT;

    TAssociativeVector(const TAssociativeVector& other);
    TAssociativeVector& operator =(const TAssociativeVector& other);

    bool operator ==(const TAssociativeVector& other) const { return _vector == other._vector; }
    bool operator !=(const TAssociativeVector& other) const { return !operator ==(other); }

    vector_type& Vector() { return _vector; }
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

    typedef TKeyIterator<iterator> key_iterator;
    typedef TKeyIterator<const_iterator> key_const_iterator;

    TIterable<key_iterator> Keys() { return MakeIterable(MakeKeyIterator(begin()), MakeKeyIterator(end())); }
    TIterable<key_const_iterator> Keys() const { return MakeIterable(MakeKeyIterator(begin()), MakeKeyIterator(end())); }

    typedef TValueIterator<iterator> value_iterator;
    typedef TValueIterator<const_iterator> value_const_iterator;

    TIterable<value_iterator> Values() { return MakeIterable(MakeValueIterator(begin()), MakeValueIterator(end())); }
    TIterable<value_const_iterator> Values() const { return MakeIterable(MakeValueIterator(begin()), MakeValueIterator(end())); }

    void reserve(size_type capacity);
    void clear();

    void clear_ReleaseMemory();

    _Value& Add(_Key&& rkey);
    _Value& Add(const _Key& key);

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
    void insert(_It&& begin, _It&& end) {
#if USE_PPE_DEBUG
        reserve(std::distance(begin, end));
        for (auto it = begin; end != it; ++it)
            Insert_AssertUnique(it->first, it->second);
#else
        _vector.insert(_vector.end(), begin, end);
#endif
    }

    void insert(_Key&& key, _Value&& value) { Insert_KeepOldIFN(std::move(key), std::move(value)); }
    void insert(const _Key& key, const _Value& value) { Insert_KeepOldIFN(key, value); }

    iterator find(const _Key& key) { return Find(key); }
    const_iterator find(const _Key& key) const { return Find(key); }

    size_t HashValue() const { return hash_value(_vector); }

    TMemoryView<value_type> MakeView() { return _vector.MakeView(); }
    TMemoryView<const value_type> MakeView() const { return _vector.MakeView(); }

    friend void swap(TAssociativeVector& lhs, TAssociativeVector& rhs) {
        swap(lhs._vector, rhs._vector);
    }

private:
    struct FKeyEqual_ : key_equal {
        bool operator ()(const value_type& val, const key_type& key) const {
            return key_equal::operator ()(val.first, key);
        }
    };

    vector_type _vector;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
hash_t hash_value(const TAssociativeVector<_Key, _Value, _EqualTo, _Vector>& associativeVector) {
    return associativeVector.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FTextWriter& operator <<(FTextWriter& oss, const TAssociativeVector<_Key, _Value, _EqualTo, _Vector>& associativeVector);
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FWTextWriter& operator <<(FWTextWriter& oss, const TAssociativeVector<_Key, _Value, _EqualTo, _Vector>& associativeVector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/AssociativeVector-inl.h"
