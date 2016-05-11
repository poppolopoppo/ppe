#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_RINGBUFFER(T, _NAME, _COUNT) \
    MALLOCA(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    Core::PodRingBuffer<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_RINGBUFFER(T, _NAME, _COUNT) \
    MALLOCA(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    Core::RingBuffer<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _IsPod = std::is_pod<T>::value >
class RingBuffer {
public:
    typedef T value_type;

    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef typename std::random_access_iterator_tag iterator_category;

    RingBuffer();
    RingBuffer(pointer storage, size_type capacity);
    explicit RingBuffer(const MemoryView<T>& storage);
    ~RingBuffer() { clear(); }

    size_type capacity() const { return _capacity; }
    size_type size() const;
    bool empty() const { return (0 == _size); }

    template <typename _Arg0, typename... _Args>
    void push_back(_Arg0&& arg0, _Args&&... args);
    template <typename _Arg0, typename... _Args>
    bool push_back_OverflowIFN(_Arg0&& arg0, _Args&&... args);

    template <typename _Arg0, typename... _Args>
    void push_front(_Arg0&& arg0, _Args&&... args);
    template <typename _Arg0, typename... _Args>
    bool push_front_OverflowIFN(_Arg0&& arg0, _Args&&... args);

    bool pop_front(pointer pvalue);
    bool pop_back(pointer pvalue);

    template <typename _Arg0, typename... _Args>
    void Queue(_Arg0&& arg0, _Args&&... args) { push_back(std::forward<_Arg0>(arg0), std::forward<_Args>(args)); }
    template <typename _Arg0, typename... _Args>
    bool Queue_OverflowIFN(_Arg0&& arg0, _Args&&... args) { push_back_OverflowIFN(std::forward<_Arg0>(arg0), std::forward<_Args>(args)); }
    bool Dequeue(pointer pvalue) { return pop_front(pvalue); }

    reference at(size_t index) { Assert(index < _size); return _storage[(_begin + index) % _capacity]; }
    const_reference at(size_t index) const { Assert(index < _size); return _storage[(_begin + index) % _capacity]; }

    reference operator [](size_t index) { return at(index); }
    const_reference operator [](size_t index) const { return at(index); }

    void clear();

    void Swap(RingBuffer& other);

private:
    size_type _begin;
    size_type _size;
    size_type _capacity;

    pointer _storage;
};
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
RingBuffer<T, _IsPod>::RingBuffer()
:   _begin(0), _size(0), _capacity(0), _storage(nullptr) {}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
RingBuffer<T, _IsPod>::RingBuffer(pointer storage, size_type capacity)
:   _begin(0), _size(0), _capacity(capacity), _storage(storage) {
    Assert(0 == _capacity || _storage);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
RingBuffer<T, _IsPod>::RingBuffer(const MemoryView<T>& storage)
:   RingBuffer(storage.Pointer(), storage.size()) {}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
auto RingBuffer<T, _IsPod>::size() const -> size_type {
    return _size;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
void RingBuffer<T, _IsPod>::push_back(_Arg0&& arg0, _Args&&... args) {
    Assert(_storage);
    Assert(_size < _capacity);

    new (&_storage[(_begin + _size++) % _capacity]) T{ std::forward<_Arg0>(arg0), std::forward<_Args>(args)... };
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
bool RingBuffer<T, _IsPod>::push_back_OverflowIFN(_Arg0&& arg0, _Args&&... args) {
    Assert(_storage);

    const bool overflow = (_size == _capacity);
    if (overflow) {
        Assert(0 < _size);
        pop_front(nullptr);
    }

    push_back(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    return overflow;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
void RingBuffer<T, _IsPod>::push_front(_Arg0&& arg0, _Args&&... args) {
    Assert(_storage);
    Assert(_size < _capacity);

    new (&_storage[(_begin + (_capacity - 1)) % _capacity]) T{ std::forward<_Arg0>(arg0), std::forward<_Args>(args)... };
    _size++;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
bool RingBuffer<T, _IsPod>::push_front_OverflowIFN(_Arg0&& arg0, _Args&&... args) {
    Assert(_storage);

    const bool overflow = (_size == _capacity);
    if (overflow) {
        Assert(0 < _size);
        pop_back(nullptr);
    }

    push_front(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    return overflow;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
bool RingBuffer<T, _IsPod>::pop_front(pointer pvalue) {
    Assert(pvalue);

    if (0 == _size)
        return false;

    Assert(_storage);
    Assert(0 < _size);
    Assert(_begin < _capacity);

    T& elt = _storage[_begin];
    if(pvalue)
        *pvalue = std::move(elt);
    if (false == _IsPod)
        elt.~T();

    _begin = ++_begin % _capacity;
    _size--;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
bool RingBuffer<T, _IsPod>::pop_back(pointer pvalue) {
    Assert(pvalue);

    if (0 == _size)
        return false;

    Assert(_storage);
    Assert(0 < _size);
    Assert(_begin < _capacity);

    T& elt = _storage[(_begin + _size - 1) % _capacity];
    if(pvalue)
        *pvalue = std::move(elt);
    if (false == _IsPod)
        elt.~T();

    _size--;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void RingBuffer<T, _IsPod>::clear() {
    if (false == _IsPod) {
        forrange(i, 0, _size)
            _storage[(_begin + i) % _capacity].~T();
    }
    _begin = _size = 0;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void RingBuffer<T, _IsPod>::Swap(RingBuffer& other) {
    std::swap(_begin,   other._begin);
    std::swap(_size,    other._size);
    std::swap(_capacity,other._capacity);
    std::swap(_storage, other._storage);
}
//----------------------------------------------------------------------------
template <typename T>
void swap(RingBuffer<T>& lhs, RingBuffer<T>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using PodRingBuffer = RingBuffer<T, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment = std::alignment_of<T>::value >
class FixedSizeRingBuffer : public RingBuffer<T> {
public:
    typedef RingBuffer<T> parent_type;

    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::const_pointer;
    using typename parent_type::reference;
    using typename parent_type::const_reference;

    using typename parent_type::size_type;
    using typename parent_type::difference_type;

    FixedSizeRingBuffer() : parent_type(reinterpret_cast<pointer>(&_insitu), _Capacity) {}

    FixedSizeRingBuffer(FixedSizeRingBuffer&& ) = delete;
    FixedSizeRingBuffer& operator =(FixedSizeRingBuffer&& rvalue) = delete;

    FixedSizeRingBuffer(const FixedSizeRingBuffer& other) = delete;
    FixedSizeRingBuffer& operator =(const FixedSizeRingBuffer& other) = delete;

    void Swap(RingBuffer<T>& other) = delete;

private:
    // /!\ won't call any ctor or dtor, values are considered as undefined
    typename ALIGNED_STORAGE(sizeof(T) * _Capacity, _Alignment) _insitu;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
