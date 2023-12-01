#pragma once

#include "Core.h"

#include "Allocator/TrackingMalloc.h"
#include "Memory/MemoryView.h"
#include "Meta/Delete.h"

#define NEW_ARRAY(_DOMAIN, T, _COUNT) \
    ::PPE::NewArray<MEMORYDOMAIN_TAG(_DOMAIN), T>(_COUNT)
#define INDIRECT_ARRAY(_DOMAIN, T, _COUNT) \
    ::PPE::TIndirectArray<MEMORYDOMAIN_TAG(_DOMAIN), T, _COUNT>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Deleter>
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

    using base_type::data;
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

    operator void* () const { return data(); }

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
    if (T* const storage = data(); !!storage)
        _Deleter::operator ()(storage, size());
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
    if (T* const storage = data(); !!storage)
        _Deleter::operator ()(storage, size());

    base_type::operator =(std::move(rvalue));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TUniqueArrayDeleter {
    void operator ()(T* arr, size_t count) const {
        Meta::Destroy(MakeView(arr, arr + count));
        tracking_free(arr);
    }
};
//----------------------------------------------------------------------------
template <typename T>
using TUniqueArray = TUniqueView<T, TUniqueArrayDeleter<T> >;
//----------------------------------------------------------------------------
template <typename _MemoryDomain, typename T, typename... _Args>
inline TUniqueArray<T> NewArray(size_t count, _Args&& ...args) {
    TUniqueArray<T> result;
    if (Likely(count > 0)) {
        T* const arr = static_cast<T*>(tracking_malloc<_MemoryDomain>(sizeof(T) * count));
        Meta::Construct(MakeView(arr, arr + count), std::forward<_Args>(args)...);
        return { arr, count };
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain, typename T, size_t _Dimension>
class TIndirectArray : public TUniqueArray<T> {
public:
    using base_type = TUniqueArray<T>;
    using base_type::operator [];

    TIndirectArray()
    :   base_type(NewArray<_MemoryDomain, T>(_Dimension))
    {}

    explicit TIndirectArray(const T& broadcast)
    :   base_type(NewArray<_MemoryDomain, T>(_Dimension, broadcast))
    {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
