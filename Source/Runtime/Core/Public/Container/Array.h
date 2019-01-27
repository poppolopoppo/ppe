#pragma once

#include "Core_fwd.h"

#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"

#include <algorithm>
#include <initializer_list>
#include <utility>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TArray {
    typedef T value_type;
    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;
    typedef Meta::TAddReference<T> reference;
    typedef Meta::TAddReference<T> const_reference;

    typedef TCheckedArrayIterator<T> iterator;
    typedef TCheckedArrayIterator<const T> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    T Data[_Dim];

    TArray() = default;
    TArray(std::initializer_list<T> v) {
        std::copy(v.begin(), v.end(), std::begin(Data));
    }

    TArray(const TArray&) = default;
    TArray& operator =(const TArray&) = default;

    TArray(TArray&&) = default;
    TArray& operator =(TArray&&) = default;

    iterator begin() { return MakeCheckedIterator(Data, 0); }
    iterator end() { return MakeCheckedIterator(Data, _Dim); }

    const_iterator begin() const { return MakeCheckedIterator(Data, 0); }
    const_iterator end() const { return MakeCheckedIterator(Data, _Dim); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return reverse_iterator(end()); }
    const_reverse_iterator rend() const { return reverse_iterator(begin()); }

    constexpr reference at(size_t index) { Assert(index < _Dim); return Data[index]; }
    constexpr const_reference at(size_t index) const { Assert(index < _Dim); return Data[index]; }

    constexpr reference operator [](size_t index) { return at(index); }
    constexpr const_reference operator [](size_t index) const { return at(index); }

    constexpr pointer data() { return (&Data[0]); }
    constexpr const_pointer data() const { return (&Data[0]); }

    TMemoryView<T> MakeView() { return PPE::MakeView(Data); }
    TMemoryView<const T> MakeView() const { return PPE::MakeView(Data); }
    TMemoryView<const T> MakeConstView() const { return PPE::MakeView(Data); }

    inline friend bool operator ==(const TArray& lhs, const TArray& rhs) {
        return std::equal(lhs.begin(), lhs.end(), rhs.Data);
    }
    inline friend bool operator !=(const TArray& lhs, const TArray& rhs) {
        return (not operator ==(lhs, rhs));
    }

    inline friend hash_t hash_value(const TArray& arr) {
        return hash_range(arr.Data, _Dim);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE