#pragma once

#include "Core/Core.h"

#include <atomic>
#include <memory>

// Multi Producer Multi Consumer Bounded Queue
// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<typename T>
class MPMCBoundedQueueView
{
public:
    ~MPMCBoundedQueueView();

    MPMCBoundedQueueView(const MPMCBoundedQueueView& ) = delete;
    MPMCBoundedQueueView& operator =(const MPMCBoundedQueueView& ) = delete;

    bool empty() const { return _enqueuePos == _dequeuePos; }
    size_t capacity() const { return _bufferMask + 1; }

    bool Enqueue(const T& data);
    bool Dequeue(T *pdata);

protected:
    struct cell_t {
        std::atomic<size_t> _sequence;
        T _data;
    };

    MPMCBoundedQueueView(cell_t *buffer, size_t bufferSize);

    cell_t *Buffer() const { return _buffer; }

private:
    typedef char cacheline_pad_t[CACHELINE_SIZE];

    cacheline_pad_t         _pad0;
    cell_t* const           _buffer;
    size_t const            _bufferMask;
    cacheline_pad_t         _pad1;
    std::atomic<size_t>     _enqueuePos;
    cacheline_pad_t         _pad2;
    std::atomic<size_t>     _dequeuePos;
    cacheline_pad_t         _pad3;
};
//----------------------------------------------------------------------------
template<typename T>
MPMCBoundedQueueView<T>::MPMCBoundedQueueView(cell_t *buffer, size_t bufferSize)
:   _buffer(buffer)
,   _bufferMask(bufferSize - 1) {
    Assert(_buffer);
    Assert( (bufferSize >= 2) &&
            ((bufferSize & (bufferSize - 1)) == 0));

    for (size_t i = 0; i < bufferSize; ++i)
        _buffer[i]._sequence.store(i, std::memory_order_relaxed);

    _enqueuePos.store(0, std::memory_order_relaxed);
    _dequeuePos.store(0, std::memory_order_relaxed);
}
//----------------------------------------------------------------------------
template<typename T>
MPMCBoundedQueueView<T>::~MPMCBoundedQueueView() {
    Assert(empty());
    Assert(_buffer);
}
//----------------------------------------------------------------------------
template<typename T>
bool MPMCBoundedQueueView<T>::Enqueue(const T& data) {
    cell_t* cell;
    size_t pos = _enqueuePos.load(std::memory_order_relaxed);

    for (;;) {
        cell = &_buffer[pos & _bufferMask];
        size_t seq = cell->_sequence.load(std::memory_order_acquire);
        intptr_t dif = (intptr_t)seq - (intptr_t)pos;
        if (dif == 0) {
            if (_enqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                break;
        }
        else if (dif < 0)
            return false;
        else
            pos = _enqueuePos.load(std::memory_order_relaxed);
    }

    cell->_data = data;
    cell->_sequence.store(pos + 1, std::memory_order_release);

    return true;
}
//----------------------------------------------------------------------------
template<typename T>
bool MPMCBoundedQueueView<T>::Dequeue(T *pdata) {
    Assert(pdata);

    cell_t* cell;
    size_t pos = _dequeuePos.load(std::memory_order_relaxed);

    for (;;) {
        cell = &_buffer[pos & _bufferMask];
        size_t seq = 
        cell->_sequence.load(std::memory_order_acquire);
        intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
        if (dif == 0) {
            if (_dequeuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                break;
        }
        else if (dif < 0)
            return false;
        else
            pos = _dequeuePos.load(std::memory_order_relaxed);
    }

    *pdata = cell->_data;
    cell->_sequence.store(pos + _bufferMask + 1, std::memory_order_release);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MPMCBoundedQueue : public MPMCBoundedQueueView<T> {
public:
    explicit MPMCBoundedQueue(size_t bufferSize);
    ~MPMCBoundedQueue();
};
//----------------------------------------------------------------------------
template <typename T>
MPMCBoundedQueue<T>::MPMCBoundedQueue(size_t bufferSize)
:   MPMCBoundedQueueView(
    reinterpret_cast<cell_t *>(aligned_malloc(bufferSize * sizeof(cell_t), CACHELINE_SIZE)),
    bufferSize ) {
    Assert(capacity() == bufferSize);
}
//----------------------------------------------------------------------------
template <typename T>
MPMCBoundedQueue<T>::~MPMCBoundedQueue() {
    cell_t *const pbuf = Buffer();
    const size_t bufferSize = capacity();
    for (size_t i = 0; i < bufferSize; ++i)
        pbuf[i].~cell_t();

    aligned_free(pbuf);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
