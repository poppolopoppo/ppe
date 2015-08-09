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
    MALLOCA(T, CONCAT(CONCAT(_, _NAME), CONCAT(_Alloca, __LINE__)), _COUNT); \
    Core::RingBuffer<T> _NAME(CONCAT(CONCAT(_, _NAME), CONCAT(_Alloca, __LINE__)).get(), _COUNT)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class RingBuffer {
public:
    typedef T value_type;
    STATIC_ASSERT(std::is_pod<T>::value); // don't support classes with dtor

    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;
    
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef typename std::random_access_iterator_tag iterator_category;

    RingBuffer();
    RingBuffer(pointer storage, size_type capacity);
    explicit RingBuffer(const MemoryView<T>& view);

    size_type size() const;
    bool empty() const { return _tail == _head; }

    template <typename _Arg0, typename... _Args>
    void Queue(_Arg0&& arg0, _Args&&... args);
    bool Dequeue(pointer pvalue);

    void clear();

    void Swap(RingBuffer& other);

private:
    size_type _head;
    size_type _tail;
    size_type _capacity;

    pointer _storage;
};
//----------------------------------------------------------------------------
template <typename T>
RingBuffer<T>::RingBuffer()
:   _head(0), _tail(0), _capacity(0), _storage(nullptr) {}
//----------------------------------------------------------------------------
template <typename T>
RingBuffer<T>::RingBuffer(pointer storage, size_type capacity)
:   _head(0), _tail(0), _capacity(capacity), _storage(storage) {
    Assert(0 == _capacity || _storage);
}
//----------------------------------------------------------------------------
template <typename T>
RingBuffer<T>::RingBuffer(const MemoryView<T>& view)
:   RingBuffer(view.Pointer(), view.size()) {}
//----------------------------------------------------------------------------
template <typename T>
auto RingBuffer<T>::size() const -> size_type {
    return (_tail > _head)
        ? (_capacity + _head) - _tail
        : _head - _tail;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename _Arg0, typename... _Args>
void RingBuffer<T>::Queue(_Arg0&& arg0, _Args&&... args) {
    Assert(_storage);
    Assert((_tail + 1) % _capacity != _head); // not full !

    _storage[_tail] = T{ std::forward<_Arg0>(arg0), std::forward<_Args>(args)... };
    _tail = (_tail + 1) % _capacity;
}
//----------------------------------------------------------------------------
template <typename T>
bool RingBuffer<T>::Dequeue(pointer pvalue) {
    Assert(pvalue);

    if (_tail == _head) 
        return false;

    Assert(_storage);

    *pvalue = std::move(_storage[_head]);
    _head = (_head + 1) % _capacity;

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
void RingBuffer<T>::clear() {
    _tail = _head = 0; // not dtor called (since T is pod)
}
//----------------------------------------------------------------------------
template <typename T>
void RingBuffer<T>::Swap(RingBuffer& other) {
    std::swap(_head, other._head);
    std::swap(_tail, other._tail);
    std::swap(_capacity, other._capacity);
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
template <typename T, size_t _Capacity, size_t _Alignment = std::alignment_of<T>::value >
class StaticRingBuffer : public RingBuffer<T> {
public:
    typedef RingBuffer<T> parent_type;

    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::const_pointer;
    using typename parent_type::reference;
    using typename parent_type::const_reference;

    using typename parent_type::size_type;
    using typename parent_type::difference_type;

    StaticRingBuffer() : parent_type(reinterpret_cast<pointer>(&_insitu), _Capacity) {}
    
    StaticRingBuffer(StaticRingBuffer&& ) = delete;
    StaticRingBuffer& operator =(StaticRingBuffer&& rvalue) = delete;

    StaticRingBuffer(const StaticRingBuffer& other) = delete;
    StaticRingBuffer& operator =(const StaticRingBuffer& other) = delete;

    void Swap(RingBuffer<T>& other) = delete;

private:
    // /!\ won't call any ctor or dtor, values are considered as undefined
    typename ALIGNED_STORAGE(sizeof(T) * _Capacity, _Alignment) _insitu;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
