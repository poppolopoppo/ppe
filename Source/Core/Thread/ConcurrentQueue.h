#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/Meta/ThreadResource.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*
// Naive concurrent queue
// https://github.com/krizhanovsky/NatSys-Lab/blob/master/lockfree_rb_q.cc
// http://natsys-lab.blogspot.ru/2013/05/lock-free-multi-producer-multi-consumer.html
*/
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T) >
class TConcurentQueue : _Allocator {
public:
    explicit TConcurentQueue(size_t capacity);
    TConcurentQueue(size_t capacity, const _Allocator& allocator);
    ~TConcurentQueue();

    TConcurentQueue(const TConcurentQueue& ) = delete;
    TConcurentQueue& operator =(const TConcurentQueue& ) = delete;

    void Produce(T&& rvalue);
    void Consume(T* pvalue);
    bool TryConsume(T* pvalue);

private:
    const size_t _capacity;

    size_t _head;
    size_t _tail;

    std::condition_variable _empty;
    std::condition_variable _overflow;

    std::mutex _barrier;

    T* _queue;
};
//----------------------------------------------------------------------------
#define CONCURRENT_QUEUE(_DOMAIN, T) \
    ::Core::TConcurentQueue<T, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*
// Naive concurrent priority queue
// The queue uses a TVector<> and can grow, based on std::push_heap()/pop_heap().
// TNot using std::priority_queue<> since it is too restrictive (no non-const reference to top, no reserve()).
*/
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T) >
class TConcurentPriorityQueue {
public:
    explicit TConcurentPriorityQueue(size_t capacity);
    TConcurentPriorityQueue(size_t capacity, const _Allocator& allocator);
    ~TConcurentPriorityQueue();

    TConcurentPriorityQueue(const TConcurentPriorityQueue& ) = delete;
    TConcurentPriorityQueue& operator =(const TConcurentPriorityQueue& ) = delete;

    void Produce(int priority, T&& rvalue); // lower is higher priority
    void Consume(T* pvalue);
    bool TryConsume(T* pvalue);

private:
    std::condition_variable _empty;
    std::mutex _barrier;

    typedef TPair<int, T> item_type;

    typedef TVector<
        item_type,
        typename _Allocator::template rebind<item_type>::other
    >   vector_type;

    struct FGreater_ {
        bool operator ()(const item_type& lhs, const item_type& rhs) const {
            return (lhs.first > rhs.first); // ordered from min to max
        }
    };

    vector_type _queue;
};
//----------------------------------------------------------------------------
#define CONCURRENT_PRIORITY_QUEUE(_DOMAIN, T) \
    ::Core::TConcurentPriorityQueue<T, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Thread/ConcurrentQueue-inl.h"
