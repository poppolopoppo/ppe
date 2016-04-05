#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Deleter = checked_deleter<T> >
class UniqueView : public MemoryView<T>, private _Deleter {
public:
    typedef MemoryView<T> base_type;

    using typename base_type::value_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::reference;
    using typename base_type::const_reference;

    using typename base_type::size_type;
    using typename base_type::difference_type;

    using typename base_type::iterator;
    using typename base_type::iterator_category;

    UniqueView();
    ~UniqueView();

    explicit UniqueView(const MemoryView<T>& other);
    UniqueView(pointer storage, size_type capacity);

    UniqueView(UniqueView&& rvalue);
    UniqueView& operator =(UniqueView&& rvalue);

    UniqueView(const UniqueView& other) = delete;
    UniqueView& operator =(const UniqueView& other) = delete;
};
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
UniqueView<T, _Deleter>::UniqueView() {}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
UniqueView<T, _Deleter>::UniqueView(const MemoryView<T>& other)
:   base_type(other) {}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
UniqueView<T, _Deleter>::UniqueView(pointer storage, size_type capacity)
:   base_type(storage, capacity) {}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
UniqueView<T, _Deleter>::~UniqueView() {
    if (base_type::data())
        _Deleter::operator ()(base_type::data());
}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
UniqueView<T, _Deleter>::UniqueView(UniqueView&& rvalue)
:   base_type(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Deleter >
auto UniqueView<T, _Deleter>::operator =(UniqueView&& rvalue) -> UniqueView& {
    if (base_type::data())
        _Deleter::operator ()(base_type::data());

    base_type::operator =(std::move(rvalue));
    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using UniqueArray = UniqueView<T, checked_deleter<T[]> >;
//----------------------------------------------------------------------------
template <typename T>
inline UniqueArray<T> NewArray(size_t count) {
    return UniqueArray<T>{ new T[count], count };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
