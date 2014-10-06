#pragma once

#include "ConcurrentQueue.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
ConcurentQueue<T, _Allocator>::ConcurentQueue(size_t capacity)
:   _capacity(capacity)
,   _head(0)
,   _tail(0) {
    _queue = _Allocator::allocate(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
ConcurentQueue<T, _Allocator>::ConcurentQueue(size_t capacity, const _Allocator& allocator)
:   _Allocator(allocator)
,   _head(0)
,   _tail(0) {
    _queue = _Allocator::allocate(_capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
ConcurentQueue<T, _Allocator>::~ConcurentQueue() {
    Assert(_queue);
    Assert(_tail == _head);

    _Allocator::deallocate(_queue, _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void ConcurentQueue<T, _Allocator>::Produce(T&& rvalue) {
    std::unique_lock<std::mutex> scopeLock(_barrier);
    _overflow.wait(scopeLock, [this] { return _tail + _capacity > _head; });

    _queue[(_head++) % _capacity] = std::move(rvalue);

    _empty.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void ConcurentQueue<T, _Allocator>::Produce(const T& value) {
    std::unique_lock<std::mutex> scopeLock(_barrier);
    _overflow.wait(scopeLock, [this] { return _tail + _capacity > _head; });

    _queue[(_head++) % _capacity] = value;

    _empty.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void ConcurentQueue<T, _Allocator>::Consume(T *pvalue) {
    Assert(pvalue);

    std::unique_lock<std::mutex> scopeLock(_barrier);
    _empty.wait(scopeLock, [this] { return _tail < _head; });

    *pvalue = std::move(_queue[(_tail++) % _capacity]);

    _overflow.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool ConcurentQueue<T, _Allocator>::TryConsume(T *pvalue) {
    Assert(pvalue);

    std::unique_lock<std::mutex> scopeLock(_barrier);

    if (_tail >= _head)
        return false;

    _empty.wait(scopeLock, [this] { return _tail < _head; });

    *pvalue = std::move(_queue[(_tail++) % _capacity]);

    _overflow.notify_one();

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
