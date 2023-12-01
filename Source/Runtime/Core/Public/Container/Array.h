#pragma once

#include "Core_fwd.h"

#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"
#include "Meta/TypeTraits.h"

#include <algorithm>
#include <initializer_list>
#include <utility>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TStaticArray {
    typedef T value_type;

    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;

    typedef Meta::TAddReference<T> reference;
    typedef Meta::TAddReference<const T> const_reference;

    typedef TCheckedArrayIterator<T> iterator;
    typedef TCheckedArrayIterator<const T> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    T Data[_Dim];

    CONSTEXPR size_t size() const { return _Dim; }

    NODISCARD iterator begin() { return MakeCheckedIterator(Data, 0); }
    NODISCARD iterator end() { return MakeCheckedIterator(Data, _Dim); }

    NODISCARD const_iterator begin() const { return MakeCheckedIterator(Data, 0); }
    NODISCARD const_iterator end() const { return MakeCheckedIterator(Data, _Dim); }

    NODISCARD reverse_iterator rbegin() { return reverse_iterator(end()); }
    NODISCARD reverse_iterator rend() { return reverse_iterator(begin()); }

    NODISCARD const_reverse_iterator rbegin() const { return reverse_iterator(end()); }
    NODISCARD const_reverse_iterator rend() const { return reverse_iterator(begin()); }

    CONSTEXPR reference at(size_t index) { Assert(index < _Dim); return Data[index]; }
    CONSTEXPR const_reference at(size_t index) const { Assert(index < _Dim); return Data[index]; }

    CONSTEXPR reference operator [](size_t index) { return at(index); }
    CONSTEXPR const_reference operator [](size_t index) const { return at(index); }

    CONSTEXPR pointer data() { return (&Data[0]); }
    CONSTEXPR const_pointer data() const { return (&Data[0]); }

    CONSTEXPR TMemoryView<T> MakeView() { return PPE::MakeView(Data); }
    CONSTEXPR TMemoryView<const T> MakeView() const { return PPE::MakeView(Data); }
    CONSTEXPR TMemoryView<const T> MakeConstView() const { return PPE::MakeView(Data); }

    template <typename U, size_t _OtherDim>
    CONSTEXPR TStaticArray<std::common_type_t<T, U>, _Dim + _OtherDim> Concat(const TStaticArray<U, _OtherDim>& other) const {
        TStaticArray<std::common_type_t<T, U>, _Dim + _OtherDim> result;
        forrange(i, 0, _Dim)
            result.Data[i] = Data[i];
        forrange(i, 0, _OtherDim)
            result.Data[_Dim + i] = other.Data[i];
        return result;
    }

    CONSTEXPR inline friend void swap(TStaticArray& lhs, TStaticArray& rhs) {
        forrange(i, 0, _Dim) {
            using namespace std;
            std::swap(lhs.Data[i], rhs.Data[i]);
        }
    }

    CONSTEXPR inline friend bool operator ==(const TStaticArray& lhs, const TStaticArray& rhs) {
        return std::equal(lhs.begin(), lhs.end(), rhs.Data);
    }
    CONSTEXPR inline friend bool operator !=(const TStaticArray& lhs, const TStaticArray& rhs) {
        return (not operator ==(lhs, rhs));
    }

    CONSTEXPR inline friend hash_t hash_value(const TStaticArray& arr) {
        return hash_mem_constexpr(arr.Data, _Dim);
    }

    CONSTEXPR inline friend size_t lengthof(const TStaticArray&) {
        return _Dim;
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR u32 lengthof(const TStaticArray<T, _Dim>& arr) { return static_cast<u32>(arr.size()); }
//----------------------------------------------------------------------------
template <typename T, typename ..._Args>
CONSTEXPR auto MakeStaticArray(_Args&&... args) {
    return TStaticArray<T, sizeof...(args)>{{ args... }};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
