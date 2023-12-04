#pragma once

#include "Thread/ConcurrentQueue.h"

#include "Meta/ThreadResource.h"

#include <algorithm>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurrentQueue<T, _Allocator>::TConcurrentQueue(size_t capacity)
:   _capacity(capacity)
,   _head(0)
,   _tail(0) {
    _queue = allocator_traits::template AllocateT<T>(*this, capacity);
    Assert(_queue);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurrentQueue<T, _Allocator>::TConcurrentQueue(size_t capacity, const _Allocator& allocator)
:   _Allocator(allocator)
,   _head(0)
,   _tail(0) {
    _queue = allocator_traits::template AllocateT<T>(*this, capacity);
    Assert(_queue);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurrentQueue<T, _Allocator>::~TConcurrentQueue() {
    Assert(_queue);
    Assert(_tail == _head);

    allocator_traits::template DeallocateT<T>(*this, _queue, _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurrentQueue<T, _Allocator>::Produce(T&& rvalue) {
    Meta::FUniqueLock scopeLock(_barrier);
    _overflow.wait(scopeLock, [this] { return _tail + _capacity > _head; });

    _queue[(_head++) % _capacity] = std::move(rvalue);

    _empty.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurrentQueue<T, _Allocator>::Consume(T *pvalue) {
    Assert(pvalue);

    Meta::FUniqueLock scopeLock(_barrier);
    _empty.wait(scopeLock, [this] { return _tail < _head; });

    *pvalue = std::move(_queue[(_tail++) % _capacity]);

    _overflow.notify_one();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool TConcurrentQueue<T, _Allocator>::TryConsume(T *pvalue) {
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
TConcurrentPriorityQueue<T, _Allocator>::TConcurrentPriorityQueue(size_t capacity)
:   _counter(0) {
    _queue.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurrentPriorityQueue<T, _Allocator>::TConcurrentPriorityQueue(size_t capacity, const _Allocator& allocator)
:   _queue(allocator)
,   _counter(0) {
    _queue.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
TConcurrentPriorityQueue<T, _Allocator>::~TConcurrentPriorityQueue() {
    Assert(0 == _counter);
    Assert_NoAssume(CheckCanary_());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void TConcurrentPriorityQueue<T, _Allocator>::Produce(u32 priority, T&& rvalue) {
    Assert_NoAssume(CheckCanary_());

    Meta::FUniqueLock scopeLock(_barrier);
    _overflow.wait(scopeLock, [this] {
        Assert_NoAssume(CheckCanary_());
        return (_queue.size() < _queue.capacity());
    });

    Assert(_counter < 0xFFFF);
    const u32 insertion_order_preserving_priority = u32((u64(priority) << 16) | _counter++);
    Assert((insertion_order_preserving_priority >> 16) == priority);

    _queue.emplace_back(insertion_order_preserving_priority, std::move(rvalue));
    std::push_heap(_queue.begin(), _queue.end(), FPrioritySort_{});

    scopeLock.unlock();  // unlock before notification to minimize mutex contention
    _empty.notify_one(); // notify one consumer thread
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
template <typename _Lambda>
void TConcurrentPriorityQueue<T, _Allocator>::Produce(u32 priority, size_t count, size_t stride, _Lambda&& lambda) {
    Assert_NoAssume(CheckCanary_());

    Assert(0 < count);
#if 1
    size_t batchIndex = 0;
    do
    {
        // time slice job pushing, prevent from flooding the queue while worker are waiting for the lock
        // also more efficient dispatch
        const size_t batchCount = Min(stride, count - batchIndex);
        {
            Meta::FUniqueLock scopeLock(_barrier);
            _overflow.wait(scopeLock, [this, batchCount] {
                Assert_NoAssume(CheckCanary_());
                return (_queue.size() + batchCount <= _queue.capacity());
            });

            Assert(_counter + batchCount < 0xFFFF);
            u32 insertion_order_preserving_priority = u32((u64(priority) << 16) | _counter);
            Assert((insertion_order_preserving_priority >> 16) == priority);

            forrange(i, 0, batchCount) {
                _queue.emplace_back(++insertion_order_preserving_priority, std::move(lambda(batchIndex + i)));
                std::push_heap(_queue.begin(), _queue.end(), FPrioritySort_{});
            }

            scopeLock.unlock();  // unlock before notification to minimize mutex contention
            _empty.notify_all(); // notify all consumer threads, assuming stride == worker thread count
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
void TConcurrentPriorityQueue<T, _Allocator>::Consume(T *pvalue) {
    Assert(pvalue);
    Assert_NoAssume(CheckCanary_());

    Meta::FUniqueLock scopeLock(_barrier);
    _empty.wait(scopeLock, [&] {
        Assert_NoAssume(CheckCanary_());
        return (not _queue.empty());
    });

    std::pop_heap(_queue.begin(), _queue.end(), FPrioritySort_{});
    *pvalue = std::move(_queue.back().second);
    _queue.pop_back();

    // reset counter when the queue is empty
    // this book-keeping should avoid counter overflow (which is checked in Produce())
    if (_queue.empty())
        _counter = 0;

    scopeLock.unlock();     // unlock before notification to minimize mutex contention
    _overflow.notify_all(); // always notifies all producer threads
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool TConcurrentPriorityQueue<T, _Allocator>::TryConsume(T *pvalue) {
    Assert(pvalue);
    Assert_NoAssume(CheckCanary_());

    Meta::FUniqueLock scopeLock(_barrier, std::defer_lock);
    if (scopeLock.try_lock() == false)
        return false; // return early if can't acquire the lock

    if (_queue.empty()) {
        Assert(0 == _counter); // already reset by last Consume()
        return false;
    }

    _empty.wait(scopeLock, [&] {
        Assert_NoAssume(CheckCanary_());
        return (not _queue.empty());
    });

    std::pop_heap(_queue.begin(), _queue.end(), FPrioritySort_{});
    *pvalue = std::move(_queue.back().second);
    _queue.pop_back();

    // reset counter when the queue is empty
    // this book-keeping should avoid counter overflow (which is checked in Produce())
    if (_queue.empty())
        _counter = 0;

    scopeLock.unlock();     // unlock before notification to minimize mutex contention
    _overflow.notify_all(); // always notifies all producer threads

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
