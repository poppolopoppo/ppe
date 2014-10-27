#pragma once

#include "Core/Thread/LockFreeCircularArray.h"

/*
// Yet another implementation of a lock-free circular array queue
// http://www.codeproject.com/Articles/153898/Yet-another-implementation-of-a-lock-free-circular?display=PrintAll
*/

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
LockFreeCircularQueue_SingleProducer<T, _Allocator>::LockFreeCircularQueue_SingleProducer(size_type capacity)
:   _capacity(capacity), _read(0), _write(0)
,   _queue(_Allocator::allocate(capacity)) {
    Assert(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
LockFreeCircularQueue_SingleProducer<T, _Allocator>::~LockFreeCircularQueue_SingleProducer() {
    Assert(_capacity);
    Assert(_queue);
    _Allocator::deallocate(_queue, _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool LockFreeCircularQueue_SingleProducer<T, _Allocator>::empty() const {
    return ((_read % _capacity) == (_write % _capacity));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
void LockFreeCircularQueue_SingleProducer<T, _Allocator>::Produce(const_reference value) {
    THIS_THREADRESOURCE_CHECKACCESS(); // only one producer allowed

    const size_type currentRead = _read;
    const size_type currentWrite = _write;

    if ((currentRead % _capacity) == ((currentWrite + 1) % _capacity) )
        Assert(false); // the queue is full

    // save the date into the q
    _queue[currentWrite % _capacity] = value;

    // increment atomically write index. Now a consumer thread can read
    // the piece of data that was just stored
    ++_write;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator >
bool LockFreeCircularQueue_SingleProducer<T, _Allocator>::Consume(reference value) {
    do {
        // _maximumRead doesn't exist when the queue is set up as
        // single-producer. The maximum read index is described by the current
        // write index
        size_type currentRead = _read;
        const size_type currentMaximumRead = _write;

        if ((currentRead % _capacity) == (currentMaximumRead % _capacity) )
        {
            // the queue is empty or
            // a producer thread has allocate space in the queue but is
            // waiting to commit the data into it
            return false;
        }

        // retrieve the data from the queue
        value = _queue[currentRead % _capacity];

        // try to perfrom now the CAS operation on the read index. If we succeed
        // value already contains what _read pointed to before we
        // increased it
        if (_read.compare_exchange_weak/* weak inside a loop is recommended by std */(currentRead, currentRead + 1) )
        {
            // got here. The value was retrieved from the queue. Note that the
            // data inside the _queue array is not deleted nor reseted
            // => And disables support for move semantics (and thus full object support instead of POD restriction)
            return true;
        }

        // it failed retrieving the element off the queue. Someone else must
        // have read the element stored at (currentRead % _capacity)
        // before we could perform the CAS operation
    } while (1); // keep looping to try again!

    // Something went wrong. it shouldn't be possible to reach here
    Assert(0);

    // Add this return statement to avoid compiler warnings
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
