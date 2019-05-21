#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Container/Hash.h"

#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RAWSTORAGE(_DOMAIN, T) \
    ::PPE::TRawStorage<T, ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
#define RAWSTORAGE_ALIGNED(_DOMAIN, T, _ALIGNMENT) \
    ::PPE::TRawStorage<T, ALIGNED_ALLOCATOR(_DOMAIN, _ALIGNMENT)>
//----------------------------------------------------------------------------
#define RAWSTORAGE_STACK(T) \
    ::PPE::TRawStorage<T, STACKLOCAL_ALLOCATOR()>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// No dtor will be called !
template <typename T, typename _Allocator = ALLOCATOR(Container)>
class TRawStorage : private _Allocator {
public:
    template <typename U, typename A>
    friend class TRawStorage;

    typedef _Allocator allocator_type;
    typedef TAllocatorTraits<_Allocator> allocator_traits;

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

    allocator_type& get_allocator() { return allocator_traits::Get(*this); }
    const allocator_type& get_allocator() const { return allocator_traits::Get(*this); }

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
        allocator_traits::Swap(*this, other);
        std::swap((void*&)_storage, (void*&)other._storage);
        std::swap(_size, other._size);
    }

    void Resize(size_type size, bool keepData);
    void clear_ReleaseMemory();

    FORCE_INLINE void Resize_DiscardData(size_type size) { Resize(size, false); }
    FORCE_INLINE void Resize_KeepData(size_type size) { Resize(size, true); }

    bool AcquireDataUnsafe(FAllocatorBlock b) NOEXCEPT {
        Assert_NoAssume(Meta::IsAligned(sizeof(value_type), b.SizeInBytes));

        if (allocator_traits::Acquire(*this, b)) {
            clear_ReleaseMemory();

            _storage = static_cast<pointer>(b.Data);
            _size = b.SizeInBytes / sizeof(value_type);

            return true;
        }
        else {
            return false;
        }
    }

    FAllocatorBlock StealDataUnsafe() NOEXCEPT {
        FAllocatorBlock b{ _storage, _size * sizeof(value_type) };
        if (allocator_traits::Steal(*this, b)) {
            // won't delete the block since it's been stolen !

            _storage = nullptr;
            _size = 0;

            return b;
        }
        else {
            return FAllocatorBlock::Null();
        }
    }

    template <typename _It>
    void insert(iterator after, _It&& begin, _It&& end);

    bool Equals(const TRawStorage& other) const;

    hash_t HashValue() const {
        return hash_as_pod_array(_storage, _size);
    }

    TMemoryView<T> SubRange(size_t offset, size_t size) {
        Assert_NoAssume(offset + size <= _size);
        return TMemoryView<T>(_storage + offset, size);
    }

    TMemoryView<const T> SubRange(size_t offset, size_t size) const {
        Assert_NoAssume(offset + size <= _size);
        return TMemoryView<T>(_storage + offset, size);
    }

    TMemoryView<T> MakeView() { return TMemoryView<T>(_storage, _size); }
    TMemoryView<const T> MakeView() const { return TMemoryView<const T>(_storage, _size); }
    TMemoryView<const T> MakeConstView() const { return TMemoryView<const T>(_storage, _size); }

    inline friend bool operator ==(const TRawStorage& lhs, const TRawStorage& rhs) { return lhs.Equals(rhs); }
    inline friend bool operator !=(const TRawStorage& lhs, const TRawStorage& rhs) { return not lhs.Equals(rhs); }

    inline friend hash_t hash_value(const TRawStorage& storage) { return storage.HashValue(); }

protected:
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
} //!namespace PPE

#include "Container/RawStorage-inl.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TRawStorage<u8>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
