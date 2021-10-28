#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Container/Hash.h"
#include "Container/RawStorage_fwd.h"
#include "Meta/Iterator.h"

#include <type_traits>

namespace PPE {
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
    typedef typename Meta::TIteratorTraits<iterator>::iterator_category iterator_category;

    TRawStorage() NOEXCEPT;
    ~TRawStorage();

    explicit TRawStorage(size_type size);
    explicit TRawStorage(allocator_type&& allocator) NOEXCEPT;
    TRawStorage(allocator_type&& allocator, size_type size);

    template <typename _It>
    TRawStorage(_It&& begin, _It&& end);
    TRawStorage(std::initializer_list<T> list) : TRawStorage(list.begin(), list.end()) {}
    TRawStorage(TMemoryView<const T> data) : TRawStorage(data.size()) { data.CopyTo(MakeView()); }

    TRawStorage(TRawStorage&& rvalue) NOEXCEPT;
    TRawStorage& operator =(TRawStorage&& rvalue) NOEXCEPT;

    TRawStorage(const TRawStorage& other);
    TRawStorage& operator =(const TRawStorage& other);

    pointer Pointer() const { return _storage; }
    size_t SizeInBytes() const { return _sizeInBytes; }

    pointer data() const { return _storage; }
    size_type size() const { return _sizeInBytes / sizeof(value_type); }
    bool empty() const { return 0 == _sizeInBytes; }

    allocator_type& get_allocator() { return allocator_traits::Get(*this); }
    const allocator_type& get_allocator() const { return allocator_traits::Get(*this); }

    iterator begin() { return _storage; }
    iterator end() { return _storage + size(); }

    const_iterator begin() const { return _storage; }
    const_iterator end() const { return _storage + size(); }

    reference front() { return at(0); }
    reference back() { return at(size() - 1); }

    const_reference front() const { return at(0); }
    const_reference back() const { return at(size() - 1); }

    reference at(size_type index);
    const_reference at(size_type index) const;

    FORCE_INLINE reference operator [](size_type index) { return at(index); }
    FORCE_INLINE const_reference operator [](size_type index) const { return at(index); }

    void CopyFrom(const TMemoryView<const T>& src);

    void Swap(TRawStorage& other) NOEXCEPT;

    template <typename U, typename A>
    typename std::enable_if< sizeof(U) == sizeof(T) >::type Swap(TRawStorage<U, A>& other) NOEXCEPT {
        allocator_traits::Swap(*this, other);
        std::swap((void*&)_storage, (void*&)other._storage);
        std::swap(_sizeInBytes, other._sizeInBytes);
    }

    void Resize(size_type size, bool keepData);
    void clear_ReleaseMemory();

    FORCE_INLINE void Resize_DiscardData(size_type size) { Resize(size, false); }
    FORCE_INLINE void Resize_KeepData(size_type size) { Resize(size, true); }

    bool AcquireDataUnsafe(FAllocatorBlock b) NOEXCEPT;
    FAllocatorBlock StealDataUnsafe() NOEXCEPT;

    template <typename _It>
    void insert(iterator after, _It&& begin, _It&& end);

    bool Equals(const TRawStorage& other) const;

    hash_t HashValue() const {
        return hash_mem(_storage, _sizeInBytes);
    }

    TMemoryView<T> SubRange(size_t offset, size_t size) {
        Assert_NoAssume(offset + size <= this->size());
        return TMemoryView<T>(_storage + offset, size);
    }

    TMemoryView<const T> SubRange(size_t offset, size_t size) const {
        Assert_NoAssume(offset + size <= this->size());
        return TMemoryView<T>(_storage + offset, size);
    }

    TMemoryView<T> MakeView() { return TMemoryView<T>(_storage, size()); }
    TMemoryView<const T> MakeView() const { return TMemoryView<const T>(_storage, size()); }
    TMemoryView<const T> MakeConstView() const { return TMemoryView<const T>(_storage, size()); }

    inline friend bool operator ==(const TRawStorage& lhs, const TRawStorage& rhs) { return lhs.Equals(rhs); }
    inline friend bool operator !=(const TRawStorage& lhs, const TRawStorage& rhs) { return not lhs.Equals(rhs); }

    inline friend hash_t hash_value(const TRawStorage& storage) { return storage.HashValue(); }

protected:
    pointer _storage;
    size_type _sizeInBytes; // /!\ we keep the size in bytes so we can reinterpret-cast TRawStorage<> to any item type
};
template <typename U, typename AU, typename V, typename AV>
void swap(TRawStorage<U, AU>& lhs, TRawStorage<V, AV>& rhs) NOEXCEPT {
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
