#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Hash.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"

#include <initializer_list>
#include <vector>

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTOR(_DOMAIN, _KEY, _VALUE) \
    ::Core::AssociativeVector<_KEY, _VALUE, std::equal_to<_KEY>, VECTOR(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>) >
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTOR_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE) \
    ::Core::AssociativeVector<_KEY, _VALUE, std::equal_to<_KEY>, VECTOR_THREAD_LOCAL(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>) >
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTORINSITU(_DOMAIN, _KEY, _VALUE, _InSituCount) \
    ::Core::AssociativeVector<_KEY, _VALUE, std::equal_to<_KEY>, VECTORINSITU(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>, _InSituCount) >
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTORINSITU_THREAD_LOCAL(_DOMAIN, _KEY, _VALUE, _InSituCount) \
    ::Core::AssociativeVector<_KEY, _VALUE, std::equal_to<_KEY>, VECTORINSITU_THREAD_LOCAL(_DOMAIN, ::Core::Pair<_KEY COMMA _VALUE>, _InSituCount) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _EqualTo = EqualTo<_Key>,
    typename _Vector = Vector<Pair<_Key COMMA _Value>> >
class AssociativeVector {
public:
    typedef Pair<_Key, _Value> value_type;
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

    AssociativeVector();
    explicit AssociativeVector(size_type capacity);
    ~AssociativeVector();

    template <typename _It>
    AssociativeVector(_It&& begin, _It&& end)
#ifdef _DEBUG
        { insert(begin, end); }
#else
        : _vector(begin, end) {}
#endif

    template <typename U>
    AssociativeVector(std::initializer_list<U> values)
        : AssociativeVector(values.begin(), values.end()) {}

    explicit AssociativeVector(vector_type&& vector);
    AssociativeVector& operator =(vector_type&& vector);

    explicit AssociativeVector(const vector_type& vector);
    AssociativeVector& operator =(const vector_type& vector);

    AssociativeVector(AssociativeVector&& rvalue);
    AssociativeVector& operator =(AssociativeVector&& rvalue);

    AssociativeVector(const AssociativeVector& other);
    AssociativeVector& operator =(const AssociativeVector& other);

    bool operator ==(const AssociativeVector& other) const { return _vector == other._vector; }
    bool operator !=(const AssociativeVector& other) const { return !operator ==(other); }

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

    void reserve(size_type capacity);
    void clear();

    void Clear_ReleaseMemory();

    iterator Find(const _Key& key);
    const_iterator Find(const _Key& key) const;

    iterator FindAfter(const _Key& key, const iterator& previous);
    const_iterator FindAfter(const _Key& key, const const_iterator& previous) const;

    bool Find(const _Key& key, _Value *pvalue) const;

    bool Insert_ReturnIfExists(_Key&& key, _Value&& rvalue);
    void Insert_KeepOldIFN(_Key&& key, _Value&& rvalue);
    void Insert_AssertUnique(_Key&& key, _Value&& rvalue);

    bool Insert_ReturnIfExists(const _Key& key, const _Value& value);
    void Insert_KeepOldIFN(const _Key& key, const _Value& value);
    void Insert_AssertUnique(const _Key& key, const _Value& value);

    _Value& Get(const _Key& key);
    bool TryGet(const _Key& key, _Value *value) const;
    const _Value& At(const _Key& key) const;

    bool Erase(const _Key& key);
    void Erase(const const_iterator& it);

    void Remove_AssertExists(const _Key& key, const _Value& valueForDebug);
    void Remove_AssertExists(const _Key& key);

    _Value& operator [](const _Key& key) { return Get(key); }
    const _Value& operator [](const _Key& key) const { return At(key); }

    template <typename _It>
    void insert(_It&& begin, _It&& end) {
#ifdef _DEBUG
        reserve(std::distance(begin, end));
        for (auto it = begin; end != it; ++it)
            Insert_AssertUnique(it->first, it->second);
#else
        _vector.insert(_vector.end(), begin, end);
#endif
    }

private:
    vector_type _vector;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
size_t hash_value(const AssociativeVector<_Key, _Value, _EqualTo, _Vector>& associativeVector) {
    return hash_value_seq(associativeVector.begin(), associativeVector.end());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key,
    typename _Value,
    typename _EqualTo,
    typename _Vector,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const AssociativeVector<_Key, _Value, _EqualTo, _Vector>& associativeVector) {
    oss << "{ ";
    for (const auto& it : associativeVector)
        oss << '(' << it.first << ", " << it.second << "), ";
    return oss << '}';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/AssociativeVector-inl.h"
