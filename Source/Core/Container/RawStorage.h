#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Hash.h"

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RAWSTORAGE(_DOMAIN, T) \
    ::Core::TRawStorage<T, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define RAWSTORAGE_ALIGNED(_DOMAIN, T, _ALIGNMENT) \
    ::Core::TRawStorage<T, ALIGNED_ALLOCATOR(_DOMAIN, T, _ALIGNMENT)>
//----------------------------------------------------------------------------
#define RAWSTORAGE_STACK(T) \
    ::Core::TRawStorage<T, STACK_ALLOCATOR(T)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// No dtor will be called !
template <typename T, typename _Allocator = ALLOCATOR(Container, T)>
class TRawStorage : _Allocator {
public:
    template <typename U, typename A>
    friend class TRawStorage;

    typedef _Allocator allocator_type;

    typedef T value_type;
    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;
    typedef Meta::TAddReference<T> reference;
    typedef Meta::TAddReference<const T> const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::random_access_iterator_tag iterator_category;

    TRawStorage();
    ~TRawStorage();

    explicit TRawStorage(size_type size);
    explicit TRawStorage(allocator_type&& allocator);
    TRawStorage(allocator_type&& allocator, size_type size);
    TRawStorage(allocator_type&& allocator, const TMemoryView<T>& stolen);

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

    const allocator_type& get_allocator() const { return (*static_cast<const allocator_type*>(this)); }

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

    template <typename U, typename A>
    typename std::enable_if< sizeof(U) == sizeof(T) >::type Swap(TRawStorage<U, A>& other) {
        AssertRelease(allocator() == other.allocator());
        std::swap((void*&)_storage, (void*&)other._storage);
        std::swap(_size, other._size);
    }

    void Resize(size_type size, bool keepData);
    void clear_ReleaseMemory();

    FORCE_INLINE void Resize_DiscardData(size_type size) { Resize(size, false); }
    FORCE_INLINE void Resize_KeepData(size_type size) { Resize(size, true); }

    template <typename _OtherAllocator>
    auto StealDataUnsafe(_OtherAllocator& alloc) {
        using other_value_type = typename _OtherAllocator::value_type;
        const TMemoryView<other_value_type> stolen = AllocatorStealBlock(alloc, MakeView(), get_allocator_());

        _storage = nullptr; // won't delete the block !
        _size = 0;
        return stolen;
    }

    template <typename _It>
    void insert(iterator after, _It&& begin, _It&& end);

    bool Equals(const TRawStorage& other) const;

    hash_t HashValue() const {
        return hash_as_pod_array(_storage, _size);
    }

    TMemoryView<T> MakeView() { return TMemoryView<T>(_storage, _size); }
    TMemoryView<const T> MakeView() const { return TMemoryView<const T>(_storage, _size); }
    TMemoryView<const T> MakeConstView() const { return TMemoryView<const T>(_storage, _size); }

    inline friend bool operator ==(const TRawStorage& lhs, const TRawStorage& rhs) { return lhs.Equals(rhs); }
    inline friend bool operator !=(const TRawStorage& lhs, const TRawStorage& rhs) { return not lhs.Equals(rhs); }

    inline friend hash_t hash_value(const TRawStorage& storage) { return storage.HashValue(); }

protected:
    allocator_type& get_allocator_() { return (static_cast<allocator_type&>(*this)); }

    pointer _storage;
    size_type _size;
};
//----------------------------------------------------------------------------
template <typename U, typename AU, typename V, typename AV>
void swap(TRawStorage<U, AU>& lhs, TRawStorage<V, AV>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/RawStorage-inl.h"
