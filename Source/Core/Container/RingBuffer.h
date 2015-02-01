#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"

#include <iterator>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity = 16>
class RingBuffer {
public:
    typedef T value_type;
    typedef POD_STORAGE(T) pod_type;
    STATIC_CONST_INTEGRAL(size_t, Capacity, _Capacity);

    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_reference<T>::type reference;
    typedef typename std::add_reference<const T>::type const_reference;
    
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef pointer iterator;
    typedef const_pointer const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef typename std::random_access_iterator_tag iterator_category;

    RingBuffer();
    ~RingBuffer();

    size_type size() const { return _tail < _head ? _head - _tail : _tail - _head; }
    bool empty() const { return _tail == _head; }

    void Produce(T&& rvalue);
    bool Consume(T *pvalue);

    void clear();

    iterator begin() { return reinterpret_cast<pointer>(&_storage[_head]); }
    iterator end() { return reinterpret_cast<pointer>(&_storage[_tail]); }

    const_iterator begin() const { return reinterpret_cast<pointer>(&_storage[_head]); }
    const_iterator end() const { return reinterpret_cast<pointer>(&_storage[_tail]); }

    reverse_iterator rbegin() { return reverse_iterator(begin()); }
    reverse_iterator rend() { return reverse_iterator(end()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(end()); }

private:
    size_type _head;
    size_type _tail;
    pod_type _storage[Capacity];
};
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
RingBuffer::RingBuffer()
:   _head(0), _tail(0) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
RingBuffer::~RingBuffer() { 
    clear(); 
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
void RingBuffer::Produce(T&& rvalue) {
    Assert((_tail + 1) % Capacity != _head); // not full !

    T *const pblock = reinterpret_cast<T *>(&_storage[_tail]);
    new ((void *)pblock) T(std::move(rvalue));

    _tail = (_tail + 1) % Capacity;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
bool RingBuffer::Consume(T *pvalue) {
    if (_tail == _head) 
        return false;

    T *const pblock = reinterpret_cast<T *>(&_storage[_head]);
    *pvalue = std::move(*pblock);
    pblock->~T();

    _head = (_head + 1) % Capacity;

    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
void RingBuffer::clear() {
    for (; _head != _tail; _head = (_head + 1) % Capacity)
        reinterpret_cast<T *>(&_storage[_head])->~T();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
