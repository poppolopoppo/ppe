#pragma once

#include "Core.h"

#include "Allocator/Alloca.h"
#include "Allocator/Allocation.h"
#include "Memory/MemoryView.h"
#include "Meta/AlignedStorage.h"

#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define RINGBUFFER(_DOMAIN, T, _COUNT) \
    PPE::TAllocatedRingBuffer<T, _COUNT, ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_RINGBUFFER(T, _NAME, _COUNT) \
    MALLOCA_POD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TPodRingBuffer<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_ASSUMEPOD_RINGBUFFER(T, _NAME, _COUNT) \
    MALLOCA_ASSUMEPOD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TPodRingBuffer<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_RINGBUFFER(T, _NAME, _COUNT) \
    MALLOCA_ASSUMEPOD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TRingBuffer<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _IsPod = Meta::is_pod_v<T> >
class TRingBuffer {
public:
    typedef T value_type;

    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;
    typedef Meta::TAddReference<T> reference;
    typedef Meta::TAddReference<const T> const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef typename std::forward_iterator_tag iterator_category;

    class FIterator {
        CONSTEXPR FIterator(const TRingBuffer& owner, size_type pos) : RingBuffer(&owner), Pos(pos) {}
    public:
        using value_type = value_type;
        using pointer = pointer;
        using reference = reference;
        using iterator_category = iterator_category;

        const TRingBuffer* RingBuffer{ nullptr };
        size_t Pos{ INDEX_NONE };

        FIterator() = default;
        FIterator(const FIterator& ) = default;
        FIterator& operator =(const FIterator& ) = default;
        FIterator(FIterator&& ) = default;
        FIterator& operator =(FIterator&& ) = default;

        FIterator& operator++() /* prefix */ { Pos = (++Pos % RingBuffer->_capacity); return (*this); }
        FIterator operator++(int) /* postfix */ {
            FIterator tmp(*this);
            ++(*this);
            return tmp;
        }

        CONSTEXPR pointer operator->() const { return std::addressof(RingBuffer->at(Pos)); }
        CONSTEXPR reference operator*() const { return RingBuffer->at(Pos); }

        CONSTEXPR bool operator ==(const FIterator& other) const { Assert(RingBuffer == other.RingBuffer);  return (Pos == other.Pos); }
        CONSTEXPR bool operator !=(const FIterator& other) const { return (not operator ==(other)); }

    };

    using iterator = FIterator;

    CONSTEXPR TRingBuffer();
    CONSTEXPR TRingBuffer(pointer storage, size_type capacity);
    CONSTEXPR explicit TRingBuffer(const TMemoryView<T>& storage);
    ~TRingBuffer() NOEXCEPT { clear(); }

    CONSTEXPR size_type capacity() const { return _capacity; }
    CONSTEXPR size_type size() const { return _size; }
    CONSTEXPR bool empty() const { return (0 == _size); }
    CONSTEXPR pointer data() const { return _storage; }

    CONSTEXPR pointer push_back_Uninitialized();
    template <typename _Arg0, typename... _Args>
    CONSTEXPR void push_back(_Arg0&& arg0, _Args&&... args);
    template <typename _Arg0, typename... _Args>
    NODISCARD CONSTEXPR bool push_back_OverflowIFN(pointer overflowIFN, _Arg0&& arg0, _Args&&... args);

    CONSTEXPR pointer push_front_Uninitialized();
    template <typename _Arg0, typename... _Args>
    CONSTEXPR void push_front(_Arg0&& arg0, _Args&&... args);
    template <typename _Arg0, typename... _Args>
    NODISCARD CONSTEXPR bool push_front_OverflowIFN(pointer overflowIFN, _Arg0&& arg0, _Args&&... args);

    NODISCARD CONSTEXPR bool pop_front(pointer pvalue);
    CONSTEXPR void pop_front_AssumeNotEmpty(pointer pvalue);
    NODISCARD CONSTEXPR bool pop_back(pointer pvalue);
    CONSTEXPR void pop_back_AssumeNotEmpty(pointer pvalue);

    template <typename _Arg0, typename... _Args>
    CONSTEXPR void Queue(_Arg0&& arg0, _Args&&... args) { push_back(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...); }
    template <typename _Arg0, typename... _Args>
    NODISCARD CONSTEXPR bool Queue_OverflowIFN(_Arg0&& arg0, _Args&&... args) { return push_back_OverflowIFN(nullptr, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...); }
    NODISCARD CONSTEXPR bool Dequeue(pointer pvalue) { return pop_front(pvalue); }

    CONSTEXPR FIterator begin() const { return FIterator(*this, _begin); }
    CONSTEXPR FIterator end() const { return FIterator(*this, (_begin + _size) % _capacity); }

    CONSTEXPR reference front() { Assert(_size); return _storage[_begin]; }
    CONSTEXPR reference back() { return at(_size - 1); }

    CONSTEXPR const_reference front() const { Assert(_size); return _storage[_begin]; }
    CONSTEXPR const_reference back() const { return at(_size - 1); }

    CONSTEXPR reference at(size_t index) { Assert(index < _size); return _storage[(_begin + index) % _capacity]; }
    CONSTEXPR const_reference at(size_t index) const { Assert(index < _size); return _storage[(_begin + index) % _capacity]; }

    CONSTEXPR reference operator [](size_t index) { return at(index); }
    CONSTEXPR const_reference operator [](size_t index) const { return at(index); }

    CONSTEXPR void clear();

    void Swap(TRingBuffer& other);

private:
    size_type _begin;
    size_type _size;
    size_type _capacity;

    pointer _storage;
};
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR TRingBuffer<T, _IsPod>::TRingBuffer()
:   _begin(0), _size(0), _capacity(0), _storage(nullptr) {}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR TRingBuffer<T, _IsPod>::TRingBuffer(pointer storage, size_type capacity)
:   _begin(0), _size(0), _capacity(capacity), _storage(storage) {
    Assert(0 == _capacity || _storage);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR TRingBuffer<T, _IsPod>::TRingBuffer(const TMemoryView<T>& storage)
:   TRingBuffer(storage.Pointer(), storage.size()) {}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR auto TRingBuffer<T, _IsPod>::push_back_Uninitialized() -> pointer {
    Assert(_storage);
    Assert(_size < _capacity);

    return (&_storage[(_begin + _size++) % _capacity]);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
CONSTEXPR void TRingBuffer<T, _IsPod>::push_back(_Arg0&& arg0, _Args&&... args) {
    Meta::Construct(push_back_Uninitialized(), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
CONSTEXPR bool TRingBuffer<T, _IsPod>::push_back_OverflowIFN(pointer overflowIFN, _Arg0&& arg0, _Args&&... args) {
    Assert(_storage);

    const bool overflow = (_size == _capacity);
    if (overflow) {
        Assert(0 < _size);
        pop_front(overflowIFN);
    }

    push_back(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    return overflow;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR auto TRingBuffer<T, _IsPod>::push_front_Uninitialized() -> pointer {
    Assert(_storage);
    Assert(_size < _capacity);

    reference slot = _storage[_begin];
    _begin = (_begin + _capacity - 1) % _capacity;
    _size++;

    return (&slot);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
CONSTEXPR void TRingBuffer<T, _IsPod>::push_front(_Arg0&& arg0, _Args&&... args) {
    Meta::Construct(push_front_Uninitialized(), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
CONSTEXPR bool TRingBuffer<T, _IsPod>::push_front_OverflowIFN(pointer overflowIFN, _Arg0&& arg0, _Args&&... args) {
    Assert(_storage);

    const bool overflow = (_size == _capacity);
    if (overflow) {
        Assert(0 < _size);
        pop_back(overflowIFN);
    }

    push_front(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    return overflow;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR bool TRingBuffer<T, _IsPod>::pop_front(pointer pvalue) {
    if (0 == _size)
        return false;

    pop_front_AssumeNotEmpty(pvalue);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR void TRingBuffer<T, _IsPod>::pop_front_AssumeNotEmpty(pointer pvalue) {
    Assert(_storage);
    Assert(0 < _size);
    Assert(_begin < _capacity);

    T& elt = _storage[_begin];
    if(pvalue)
        *pvalue = std::move(elt);

    Meta::Destroy(&elt);

    _begin = ++_begin % _capacity;
    _size--;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR bool TRingBuffer<T, _IsPod>::pop_back(pointer pvalue) {
    if (0 == _size)
        return false;

    pop_back_AssumeNotEmpty(pvalue);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR void TRingBuffer<T, _IsPod>::pop_back_AssumeNotEmpty(pointer pvalue) {
    Assert(_storage);
    Assert(0 < _size);
    Assert(_begin < _capacity);

    T& elt = _storage[(_begin + _size - 1) % _capacity];
    if(pvalue)
        *pvalue = std::move(elt);

    Meta::Destroy(&elt);

    _size--;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
CONSTEXPR void TRingBuffer<T, _IsPod>::clear() {
    IF_CONSTEXPR(false == _IsPod) {
        forrange(i, 0, _size)
            Meta::Destroy(&_storage[(_begin + i) % _capacity]);
    }
    _begin = _size = 0;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void TRingBuffer<T, _IsPod>::Swap(TRingBuffer& other) {
    std::swap(_begin,   other._begin);
    std::swap(_size,    other._size);
    std::swap(_capacity,other._capacity);
    std::swap(_storage, other._storage);
}
//----------------------------------------------------------------------------
template <typename T>
void swap(TRingBuffer<T>& lhs, TRingBuffer<T>& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TPodRingBuffer = TRingBuffer<T, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment = std::alignment_of<T>::value >
class TFixedSizeRingBuffer : public TRingBuffer<T> {
public:
    typedef TRingBuffer<T> parent_type;

    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::const_pointer;
    using typename parent_type::reference;
    using typename parent_type::const_reference;

    using typename parent_type::size_type;
    using typename parent_type::difference_type;

    CONSTEXPR TFixedSizeRingBuffer() : parent_type(reinterpret_cast<pointer>(&_insitu), _Capacity) {}

    TFixedSizeRingBuffer(TFixedSizeRingBuffer&& ) = delete;
    TFixedSizeRingBuffer& operator =(TFixedSizeRingBuffer&& rvalue) = delete;

    TFixedSizeRingBuffer(const TFixedSizeRingBuffer& other) = delete;
    TFixedSizeRingBuffer& operator =(const TFixedSizeRingBuffer& other) = delete;

    void Swap(TRingBuffer<T>& other) = delete;

private:
    // /!\ won't call any ctor or dtor, values are considered as undefined
    typename ALIGNED_STORAGE(sizeof(T) * _Capacity, _Alignment) _insitu;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _DefaultCapacity = 0, typename _Allocator = ALLOCATOR(Container) >
class TAllocatedRingBuffer : _Allocator, public TRingBuffer<T>, Meta::FNonCopyableNorMovable {
public:
    typedef TRingBuffer<T> parent_type;
    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::const_pointer;
    using typename parent_type::reference;
    using typename parent_type::const_reference;

    using typename parent_type::size_type;
    using typename parent_type::difference_type;

    explicit TAllocatedRingBuffer(size_t capacity = _DefaultCapacity)
    :   parent_type(allocator_traits::template AllocateT<T>(static_cast<_Allocator&>(*this), capacity), capacity)
    {}
    TAllocatedRingBuffer(size_t capacity, allocator_type&& rallocator)
    :   allocator_type(std::move(rallocator))
    ,   parent_type(allocator_traits::template AllocateT<T>(static_cast<_Allocator&>(*this), capacity), capacity)
    {}

    ~TAllocatedRingBuffer() {
        allocator_traits::template DeallocateT<T>(static_cast<_Allocator&>(*this), parent_type::data(), parent_type::capacity());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
