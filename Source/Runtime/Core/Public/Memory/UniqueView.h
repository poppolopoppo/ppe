#pragma once

#include "Core.h"

#include "Memory/MemoryView.h"
#include "Meta/Delete.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Deleter = checked_deleter<T> >
class TUniqueView : public TMemoryView<T>, private _Deleter {
public:
    typedef TMemoryView<T> base_type;

    using typename base_type::value_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::reference;
    using typename base_type::const_reference;

    using typename base_type::size_type;
    using typename base_type::difference_type;

    using typename base_type::iterator;
    using typename base_type::iterator_category;

    using base_type::empty;
    using base_type::size;
    using base_type::operator [];

    TUniqueView() NOEXCEPT;
    ~TUniqueView();

    explicit TUniqueView(const TMemoryView<T>& other);
    TUniqueView(pointer storage, size_type capacity);

    TUniqueView(TUniqueView&& rvalue) NOEXCEPT;
    TUniqueView& operator =(TUniqueView&& rvalue) NOEXCEPT;

    TUniqueView(const TUniqueView& other) = delete;
    TUniqueView& operator =(const TUniqueView& other) = delete;

    operator void* () const { return base_type::data(); }

    void Reset(TUniqueView&& rvalue);
};
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
TUniqueView<T, _Deleter>::TUniqueView() NOEXCEPT
{}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
TUniqueView<T, _Deleter>::TUniqueView(const TMemoryView<T>& other)
:   base_type(other) {}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
TUniqueView<T, _Deleter>::TUniqueView(pointer storage, size_type capacity)
:   base_type(storage, capacity) {}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
TUniqueView<T, _Deleter>::~TUniqueView() {
    if (base_type::data())
        _Deleter::operator ()(base_type::data());
}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
TUniqueView<T, _Deleter>::TUniqueView(TUniqueView&& rvalue) NOEXCEPT
:   base_type(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
auto TUniqueView<T, _Deleter>::operator =(TUniqueView&& rvalue) NOEXCEPT -> TUniqueView& {
    Reset(std::move(rvalue));
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
void TUniqueView<T, _Deleter>::Reset(TUniqueView&&  rvalue) {
    if (base_type::data())
        _Deleter::operator ()(base_type::data());

    base_type::operator =(std::move(rvalue));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TUniqueArray = TUniqueView<T, checked_deleter<T[]> >;
//----------------------------------------------------------------------------
template <typename T>
inline TUniqueArray<T> NewArray(size_t count) {
    return (count
        ? TUniqueArray<T>{ new (std::nothrow) T[count], count }
        : TUniqueArray<T>{ nullptr, 0 } );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
