#pragma once

#include "Core.h"
#include "Allocation.h"
#include "ThreadResource.h"

#include <condition_variable>
#include <mutex>
#include <type_traits>

/*
// Naive concurrent queue
// https://github.com/krizhanovsky/NatSys-Lab/blob/master/lockfree_rb_q.cc
// http://natsys-lab.blogspot.ru/2013/05/lock-free-multi-producer-multi-consumer.html
*/

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T) >
class ConcurentQueue : _Allocator {
public:
    STATIC_ASSERT(std::is_pod<T>::value);

    explicit ConcurentQueue(size_t capacity);
    ConcurentQueue(size_t capacity, const _Allocator& allocator);
    ~ConcurentQueue();

    ConcurentQueue(const ConcurentQueue& ) = delete;
    ConcurentQueue& operator =(const ConcurentQueue& ) = delete;

    void Produce(T&& rvalue);
    void Produce(const T& value);

    void Consume(T *pvalue);
    bool TryConsume(T *pvalue);

private:
    const size_t _capacity;

    size_t _head;
    size_t _tail;

    std::condition_variable    _empty;
    std::condition_variable    _overflow;

    std::mutex _barrier;

    T *_queue;
};
//----------------------------------------------------------------------------
#define CONCURRENT_QUEUE(_DOMAIN, T) \
    ::Core::ConcurentQueue<T, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "ConcurrentQueue-inl.h"
