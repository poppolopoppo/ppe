#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RAWSTORAGE(_DOMAIN, T) \
    ::Core::TRawStorage<T, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define RAWSTORAGE_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::TRawStorage<T, THREAD_LOCAL_ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define RAWSTORAGE_ALIGNED(_DOMAIN, T, _ALIGNMENT) \
    ::Core::TRawStorage<T, ALIGNED_ALLOCATOR(_DOMAIN, T, _ALIGNMENT)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// No dtor will be called !
template <typename T, typename _Allocator = ALLOCATOR(Container, T)>
class TRawStorage : _Allocator {
public:
    typedef _Allocator allocator_type;

    typedef T value_type;
    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::random_access_iterator_tag iterator_category;

    TRawStorage();
    ~TRawStorage();

    explicit TRawStorage(size_type size);
    explicit TRawStorage(allocator_type&& allocator);
    TRawStorage(size_type size, allocator_type&& allocator);

    template <typename _It>
    TRawStorage(_It&& begin, _It&& end);

    TRawStorage(TRawStorage&& rvalue);
    TRawStorage& operator =(TRawStorage&& rvalue);

    TRawStorage(const TRawStorage& other);
    TRawStorage& operator =(const TRawStorage& other);

    pointer Pointer() const { return _storage; }
    size_t SizeInBytes() const { return _size * sizeof(T); }

    pointer data() const { return _storage; }
    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }

    iterator begin() { return _storage; }
    iterator end() { return _storage + _size; }

    const_iterator begin() const { return _storage; }
    const_iterator end() const { return _storage + _size; }

    reference front() { return at(0); }
    reference back() { return at(_size - 1); }

    const_reference front() const { return at(0); }
    const_reference back() const { return at(_size - 1); }

    reference at(size_type index);
    const_reference at(size_type index) const;

    FORCE_INLINE reference operator [](size_type index) { return at(index); }
    FORCE_INLINE const_reference operator [](size_type index) const { return at(index); }

    void CopyFrom(const TMemoryView<const T>& src);

    void Swap(TRawStorage& other);

    void Resize(size_type size, bool keepData);
    void Clear_ReleaseMemory();

    FORCE_INLINE void Resize_DiscardData(size_type size) { Resize(size, false); }
    FORCE_INLINE void Resize_KeepData(size_type size) { Resize(size, true); }

    void Clear_StealData(pointer p, size_type size);

    template <typename _It>
    void insert(iterator after, _It&& begin, _It&& end);

    bool Equals(const TRawStorage& other) const;

    TMemoryView<T> MakeView() { return TMemoryView<T>(data(), size()); }
    TMemoryView<const T> MakeView() const { return TMemoryView<const T>(data(), size()); }
    TMemoryView<const T> MakeConstView() const { return TMemoryView<const T>(data(), size()); }

    friend bool operator ==(const TRawStorage& lhs, const TRawStorage& rhs) { return lhs.Equals(rhs); }
    friend bool operator !=(const TRawStorage& lhs, const TRawStorage& rhs) { return not lhs.Equals(rhs); }

protected:
    pointer _storage;
    size_type _size;
};
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void swap(TRawStorage<T, _Allocator>& lhs, TRawStorage<T, _Allocator>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/RawStorage-inl.h"
