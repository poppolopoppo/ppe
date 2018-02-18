#pragma once

#include "Core/Thread/ConcurrentQueue.h"
#include "Core/Meta/ThreadResource.h"

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
    Meta::FUniqueLock scopeLock(_barrier);
    _overflow.wait(scopeLock, [this] { return _tail + _capacity > _head; });

    _queue[(_head++) % _capacity] = std::move(rvalue);

    _empty.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurentQueue<T, _Allocator>::Consume(T *pvalue) {
    Assert(pvalue);

    Meta::FUniqueLock scopeLock(_barrier);
    _empty.wait(scopeLock, [this] { return _tail < _head; });

    *pvalue = std::move(_queue[(_tail++) % _capacity]);

    _overflow.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool TConcurentQueue<T, _Allocator>::TryConsume(T *pvalue) {
    Assert(pvalue);

    Meta::FUniqueLock scopeLock(_barrier);

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
TConcurentPriorityQueue<T, _Allocator>::TConcurentPriorityQueue(size_t capacity) 
:   _counter(0) 
,   _capacity(capacity) {
    Assert(_capacity > 0);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurentPriorityQueue<T, _Allocator>::TConcurentPriorityQueue(size_t capacity, const _Allocator& allocator)
:   _queue(allocator)
,   _counter(0) 
,   _capacity(capacity) {
    Assert(_capacity > 0);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurentPriorityQueue<T, _Allocator>::~TConcurentPriorityQueue() {
    Assert(0 == _counter);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurentPriorityQueue<T, _Allocator>::Produce(u32 priority, T&& rvalue) {
    Meta::FUniqueLock scopeLock(_barrier);
    _overflow.wait(scopeLock, [this] { return (_queue.size() < _capacity); });

    Assert(_counter < 0xFFFF);
    const u32 insertion_order_preserving_priority = u32((u64(priority) << 16) | _counter++);
    Assert((insertion_order_preserving_priority >> 16) == priority);

    _queue.emplace(insertion_order_preserving_priority, std::move(rvalue));
    _empty.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
template <typename _Lambda>
void TConcurentPriorityQueue<T, _Allocator>::Produce(u32 priority, size_t count, size_t stride, _Lambda&& lambda) {
    Assert(0 < count);
#if 1
    size_t batchIndex = 0;
    do
    {
        // time slice job pushing, prevent from flooding the queue while worker are waiting for the lock
        const size_t batchCount = Min(stride, count - batchIndex);
        {
            Meta::FUniqueLock scopeLock(_barrier);
            _overflow.wait(scopeLock, [this, batchCount] { return (_queue.size() + batchCount <= _capacity); });

            Assert(_counter + batchCount < 0xFFFF);
            u32 insertion_order_preserving_priority = u32((u64(priority) << 16) | _counter);
            Assert((insertion_order_preserving_priority >> 16) == priority);

            forrange(i, 0, batchCount)
                _queue.emplace(++insertion_order_preserving_priority, std::move(lambda(batchIndex + i)));

            // notify outside the loop when everything is ready
            forrange(i, 0, batchCount)
                _empty.notify_one();
        }
        batchIndex += batchCount;
    }
    while (batchIndex != count);
#else
    forrange(i, 0, count)
        Produce(priority, lambda(i));
#endif
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurentPriorityQueue<T, _Allocator>::Consume(T *pvalue) {
    Assert(pvalue);

    Meta::FUniqueLock scopeLock(_barrier);
    _empty.wait(scopeLock, [&] { return (not _queue.empty()); });

    *pvalue = std::move(_queue.top().second);
    _queue.pop();

    // reset counter when the queue is empty
    // this book-keeping should avoid counter overflow (which is checked in Produce())
    if (_queue.empty())
        _counter = 0;

    _overflow.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool TConcurentPriorityQueue<T, _Allocator>::TryConsume(T *pvalue) {
    Assert(pvalue);

    Meta::FUniqueLock scopeLock(_barrier);

    if (_queue.empty()) {
        Assert(0 == _counter); // already reset by last Consume()
        return false;
    }

    _empty.wait(scopeLock, [&] { return (not _queue.empty()); });

    *pvalue = std::move(_queue.top().second);
    _queue.pop();

    // reset counter when the queue is empty
    // this book-keeping should avoid counter overflow (which is checked in Produce())
    if (_queue.empty())
        _counter = 0;

    _overflow.notify_one();

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
