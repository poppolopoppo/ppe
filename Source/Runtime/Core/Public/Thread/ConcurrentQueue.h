#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Container/Pair.h"
#include "Container/Vector.h"
#include "Meta/ThreadResource.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <type_traits>

#define CONCURRENT_QUEUE(_DOMAIN, T) \
    ::PPE::TConcurrentQueue<T, ALLOCATOR(_DOMAIN)>
#define CONCURRENT_PRIORITY_QUEUE(_DOMAIN, T) \
    ::PPE::TConcurrentPriorityQueue<T, ALLOCATOR(_DOMAIN)>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*
// Naive concurrent queue
// https://github.com/krizhanovsky/NatSys-Lab/blob/master/lockfree_rb_q.cc
// http://natsys-lab.blogspot.ru/2013/05/lock-free-multi-producer-multi-consumer.html
*/
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container) >
class TConcurrentQueue : _Allocator {
public:
    explicit TConcurrentQueue(size_t capacity);
    TConcurrentQueue(size_t capacity, const _Allocator& allocator);
    ~TConcurrentQueue();

    TConcurrentQueue(const TConcurrentQueue& ) = delete;
    TConcurrentQueue& operator =(const TConcurrentQueue& ) = delete;

    void Produce(T&& rvalue);
    void Consume(T* pvalue);
    bool TryConsume(T* pvalue);

private:
    using allocator_traits = TAllocatorTraits< _Allocator >;

    const size_t _capacity;

    size_t _head;
    size_t _tail;

    std::condition_variable _empty;
    std::condition_variable _overflow;

    std::mutex _barrier;

    T* _queue;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*
// Naive concurrent priority queue
// The queue uses a TVector<> and can grow, based on std::push_heap()/pop_heap().
// TNot using std::priority_queue<> since it is too restrictive (no non-const reference to top, no reserve()).
*/
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container) >
class TConcurrentPriorityQueue {
public:
    explicit TConcurrentPriorityQueue(size_t capacity);
    TConcurrentPriorityQueue(size_t capacity, const _Allocator& allocator);
    ~TConcurrentPriorityQueue();

    TConcurrentPriorityQueue(const TConcurrentPriorityQueue& ) = delete;
    TConcurrentPriorityQueue& operator =(const TConcurrentPriorityQueue& ) = delete;

    // no lock for empty, should be ok but be careful when you use this
    bool empty() const { return _queue.empty(); }

    // lower is higher priority
    void Produce(u32 priority, T&& rvalue);
    template <typename _Lambda>
    void Produce(u32 priority, size_t count, size_t stride, _Lambda&& lambda);

    void Consume(T* pvalue);
    bool TryConsume(T* pvalue);

private:
    typedef TPair<u32, T> item_type;

    struct FPrioritySort_ {
        bool operator ()(const item_type& lhs, const item_type& rhs) const {
            return (lhs.first > rhs.first);
        }
    };


#if USE_PPE_ASSERT
    STATIC_CONST_INTEGRAL(size_t, CanaryDefault, CODE3264(0xDEADBEEFul, 0xDEADBEEFDEADBEEFul));
    const size_t _canary0 = CanaryDefault;
#endif

    std::mutex _barrier;

    std::condition_variable _empty;
    std::condition_variable _overflow;

    TVector<item_type, _Allocator> _queue;
    size_t _counter;

#if USE_PPE_ASSERT
    const size_t _canary1 = CanaryDefault;
    bool CheckCanary_() const {
        return (_canary0 == CanaryDefault &&
                _canary1 == CanaryDefault );
    }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Thread/ConcurrentQueue-inl.h"
