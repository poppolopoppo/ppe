#pragma once

#include "Core.h"
#include "Allocation.h"
#include "ThreadResource.h"

#include <atomic>
#include <type_traits>

/*
// Yet another implementation of a lock-free circular array queue
// http://www.codeproject.com/Articles/153898/Yet-another-implementation-of-a-lock-free-circular?display=PrintAll
*/

/*
// TODO: Lock-free Multi-producer Multi-consumer Queue on Ring Buffer ?
// https://github.com/krizhanovsky/NatSys-Lab/blob/master/lockfree_rb_q.cc
// http://natsys-lab.blogspot.ru/2013/05/lock-free-multi-producer-multi-consumer.html
*/

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T) >
class LockFreeCircularQueue_SingleProducer : public Meta::ThreadResource, _Allocator {
public:
    static_assert(std::is_pod<T>::value, "T must be a pod type"); // ctor/dtor are not supported !

    typedef T value_type;
    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_reference<T>::type reference;
    typedef typename std::add_reference<const T>::type const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    explicit LockFreeCircularQueue_SingleProducer(size_type capacity);
    ~LockFreeCircularQueue_SingleProducer();

    LockFreeCircularQueue_SingleProducer(LockFreeCircularQueue_SingleProducer&&) = delete;
    LockFreeCircularQueue_SingleProducer& operator =(LockFreeCircularQueue_SingleProducer&&) = delete;

    LockFreeCircularQueue_SingleProducer(const LockFreeCircularQueue_SingleProducer&) = delete;
    LockFreeCircularQueue_SingleProducer& operator =(const LockFreeCircularQueue_SingleProducer&) = delete;

    bool empty() const;

    void Produce(const_reference value);
    bool Consume(reference value);

private:
    pointer const _queue;
    const size_type _capacity;

#pragma warning( push )
#pragma warning( disable : 4324 ) // warning C4324: 'XXX' : structure was padded due to __declspec(align())
    // my 2 cents about false cache line sharing :
    CACHELINE_ALIGNED std::atomic<size_type> _read;
    CACHELINE_ALIGNED std::atomic<size_type> _write;
#pragma warning( pop )
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "LockFreeCircularArray-inl.h"
