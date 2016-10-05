#pragma once

#include "Core/Thread/ConcurrentQueue.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurentQueue<T, _Allocator>::TConcurentQueue(size_t capacity)
:   _capacity(capacity)
,   _head(0)
,   _tail(0) {
    _queue = _Allocator::allocate(capacity);
    Assert(_queue);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurentQueue<T, _Allocator>::TConcurentQueue(size_t capacity, const _Allocator& allocator)
:   _Allocator(allocator)
,   _head(0)
,   _tail(0) {
    _queue = _Allocator::allocate(_capacity);
    Assert(_queue);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurentQueue<T, _Allocator>::~TConcurentQueue() {
    Assert(_queue);
    Assert(_tail == _head);

    _Allocator::deallocate(_queue, _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurentQueue<T, _Allocator>::Produce(T&& rvalue) {
    std::unique_lock<std::mutex> scopeLock(_barrier);
    _overflow.wait(scopeLock, [this] { return _tail + _capacity > _head; });

    _queue[(_head++) % _capacity] = std::move(rvalue);

    _empty.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurentQueue<T, _Allocator>::Consume(T *pvalue) {
    Assert(pvalue);

    std::unique_lock<std::mutex> scopeLock(_barrier);
    _empty.wait(scopeLock, [this] { return _tail < _head; });

    *pvalue = std::move(_queue[(_tail++) % _capacity]);

    _overflow.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool TConcurentQueue<T, _Allocator>::TryConsume(T *pvalue) {
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
template <typename T, typename _Allocator >
TConcurentPriorityQueue<T, _Allocator>::TConcurentPriorityQueue(size_t capacity) {
    _queue.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurentPriorityQueue<T, _Allocator>::TConcurentPriorityQueue(size_t capacity, const _Allocator& allocator)
:   _queue(allocator) {
    _queue.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurentPriorityQueue<T, _Allocator>::~TConcurentPriorityQueue() {}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurentPriorityQueue<T, _Allocator>::Produce(int priority, T&& rvalue) {
    std::unique_lock<std::mutex> scopeLock(_barrier);

    _queue.emplace_back(priority, std::move(rvalue));
    std::push_heap(_queue.begin(), _queue.end(), FGreater_());

    _empty.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurentPriorityQueue<T, _Allocator>::Consume(T *pvalue) {
    Assert(pvalue);

    std::unique_lock<std::mutex> scopeLock(_barrier);
    _empty.wait(scopeLock, [this] { return (not _queue.empty()); });

    *pvalue = std::move(_queue.front().second);

    std::pop_heap(_queue.begin(), _queue.end(), FGreater_());
    _queue.pop_back();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool TConcurentPriorityQueue<T, _Allocator>::TryConsume(T *pvalue) {
    Assert(pvalue);

    std::unique_lock<std::mutex> scopeLock(_barrier);

    if (_queue.empty())
        return false;

    _empty.wait(scopeLock, [this] { return (not _queue.empty()); });

    *pvalue = std::move(_queue.front().second);

    std::pop_heap(_queue.begin(), _queue.end(), FGreater_());
    _queue.pop_back();

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
