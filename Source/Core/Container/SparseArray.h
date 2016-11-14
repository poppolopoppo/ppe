#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

#include <type_traits>

// TODO : DataArray<T> http://greysphere.tumblr.com/

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_SPARSEARRAY(T, _NAME, _COUNT) \
    MALLOCA(Core::TSparseArraySlot<T>, CONCAT(_Alloca_, _NAME), _COUNT); \
    Core::TPodSparseArray<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_SPARSEARRAY(T, _NAME, _COUNT) \
    MALLOCA(Core::TSparseArraySlot<T>, CONCAT(_Alloca_, _NAME), _COUNT); \
    Core::TSparseArray<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TSparseArraySlot {
public:
    typedef T value_type;

    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef size_type FDataID;

    T Element;
    FDataID Id;
};
//----------------------------------------------------------------------------
template <typename T, bool _IsPod = std::is_pod<T>::value >
class TSparseArray {
public:
    typedef TSparseArraySlot<T> FSlot;
    typedef typename FSlot::FDataID FDataID;

    typedef typename FSlot::value_type value_type;
    typedef typename FSlot::pointer pointer;
    typedef typename FSlot::const_pointer const_pointer;
    typedef typename FSlot::reference reference;
    typedef typename FSlot::const_reference const_reference;

    typedef typename FSlot::size_type size_type;
    typedef typename FSlot::difference_type difference_type;

    TSparseArray();
    TSparseArray(FSlot* storage, size_type capacity);
    explicit TSparseArray(const TMemoryView<FSlot>& storage);
    ~TSparseArray() { clear(); }

    size_type capacity() const { return _capacity; }
    size_type size() const { return _size; }
    bool empty() const { return (0 == _size); }

    void clear();

    reference Alloc();
    void Free(reference elt);

    FDataID GetID(const_reference elt) const;

    reference Get(FDataID id) const;
    pointer GetIFP(FDataID id) const;

    bool Next(pointer& pelt) const;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TPodSparseArray = TSparseArray<T, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment = std::alignment_of<T>::value >
class TFixedSizeSparseArray : public TSparseArray<T> {
public:
    typedef TSparseArray<T> parent_type;

    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::const_pointer;
    using typename parent_type::reference;
    using typename parent_type::const_reference;

    using typename parent_type::size_type;
    using typename parent_type::difference_type;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
