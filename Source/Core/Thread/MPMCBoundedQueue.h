#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"

#include <atomic>
#include <memory>

// Multi Producer Multi Consumer Bounded Queue
// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<typename T>
class MPMCBoundedQueueView {
public:
    typedef T value_type;
    typedef size_t size_type;

    ~MPMCBoundedQueueView();

    MPMCBoundedQueueView(const MPMCBoundedQueueView& ) = delete;
    MPMCBoundedQueueView& operator =(const MPMCBoundedQueueView& ) = delete;

    bool empty() const { return _enqueuePos == _dequeuePos; }
    size_type capacity() const { return _bufferMask + 1; }

    bool Produce(T&& rdata);
    bool Consume(T *pdata);

protected:
    struct cell_t {
        std::atomic<size_type> _sequence;
        typename POD_STORAGE(T) _data;
    };

    MPMCBoundedQueueView();
    MPMCBoundedQueueView(cell_t *buffer, size_type bufferSize);

    cell_t *Buffer() const { return _buffer; }

private:
    typedef char cacheline_pad_t[CACHELINE_SIZE];

    cacheline_pad_t         _pad0;
    cell_t* const           _buffer;
    size_type const         _bufferMask;
    cacheline_pad_t         _pad1;
    std::atomic<size_type>  _enqueuePos;
    cacheline_pad_t         _pad2;
    std::atomic<size_type>  _dequeuePos;
    cacheline_pad_t         _pad3;
};
//----------------------------------------------------------------------------
template<typename T>
MPMCBoundedQueueView<T>::MPMCBoundedQueueView()
:   _buffer(nullptr)
,   _bufferMask(0) {
    _enqueuePos.store(0, std::memory_order_relaxed);
    _dequeuePos.store(0, std::memory_order_relaxed);
}
//----------------------------------------------------------------------------
template<typename T>
MPMCBoundedQueueView<T>::MPMCBoundedQueueView(cell_t *buffer, size_type bufferSize)
:   _buffer(buffer)
,   _bufferMask(bufferSize - 1) {
    Assert(_buffer);
    Assert( (bufferSize >= 2) &&
            ((bufferSize & (bufferSize - 1)) == 0));

    for (size_type i = 0; i < bufferSize; ++i)
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
bool MPMCBoundedQueueView<T>::Produce(T&& rdata) {
    cell_t* cell;
    size_type pos = _enqueuePos.load(std::memory_order_relaxed);

    for (;;) {
        cell = &_buffer[pos & _bufferMask];
        size_type seq = cell->_sequence.load(std::memory_order_acquire);
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

    new ((void*)&cell->_data) T(std::move(rdata));
    cell->_sequence.store(pos + 1, std::memory_order_release);

    return true;
}
//----------------------------------------------------------------------------
template<typename T>
bool MPMCBoundedQueueView<T>::Consume(T *pdata) {
    Assert(pdata);

    cell_t* cell;
    size_type pos = _dequeuePos.load(std::memory_order_relaxed);

    for (;;) {
        cell = &_buffer[pos & _bufferMask];
        size_type seq =
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

    *pdata = std::move(*reinterpret_cast<T*>(&cell->_data));
    reinterpret_cast<T*>(&cell->_data)->~T();
    cell->_sequence.store(pos + _bufferMask + 1, std::memory_order_release);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MPMCBoundedQueue : public MPMCBoundedQueueView<T> {
public:
    typedef MPMCBoundedQueueView<T> parent_type;

    using typename parent_type::cell_t;
    using typename parent_type::size_type;

    using parent_type::Buffer;
    using parent_type::capacity;

    MPMCBoundedQueue();
    explicit MPMCBoundedQueue(size_type bufferSize);
    ~MPMCBoundedQueue();
};
//----------------------------------------------------------------------------
template <typename T>
MPMCBoundedQueue<T>::MPMCBoundedQueue()
:   MPMCBoundedQueueView<T>(nullptr, 0)
{
    Assert(capacity() == 0);
}
//----------------------------------------------------------------------------
template <typename T>
MPMCBoundedQueue<T>::MPMCBoundedQueue(size_type bufferSize)
:   MPMCBoundedQueueView<T>(reinterpret_cast<cell_t *>(aligned_malloc(bufferSize * sizeof(cell_t), CACHELINE_SIZE)),
                            bufferSize )
{
    Assert(capacity() == bufferSize);
}
//----------------------------------------------------------------------------
template <typename T>
MPMCBoundedQueue<T>::~MPMCBoundedQueue() {
    cell_t *const pbuf = Buffer();
    const size_type bufferSize = capacity();
    for (size_type i = 0; i < bufferSize; ++i)
        pbuf[i].~cell_t();

    aligned_free(pbuf);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
class MPMCFixedSizeQueue : public MPMCBoundedQueueView<T> {
public:
    typedef MPMCBoundedQueueView<T> parent_type;

    using typename parent_type::cell_t;
    using typename parent_type::size_type;

    STATIC_CONST_INTEGRAL(size_type, Capacity, _Capacity);

    MPMCFixedSizeQueue();

    explicit MPMCFixedSizeQueue(size_type bufferSize) // compatibility with MPMCBoundedQueue<T>
        : MPMCFixedSizeQueue() {
        UNUSED(bufferSize);
    }

private:
    typename POD_STORAGE(T(&)[_Capacity]) _storage;
};
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
MPMCFixedSizeQueue<T, _Capacity>::MPMCFixedSizeQueue()
:   MPMCBoundedQueueView<T>(reinterpret_cast<cell_t *>(&_storage), _Capacity)
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MPMCQueue, size_t _MaxPriority>
class MPMCPriorityQueue {
public:
    typedef _MPMCQueue queue_type;

    typedef typename queue_type::value_type value_type;
    typedef typename queue_type::size_type size_type;

    MPMCPriorityQueue();
    explicit MPMCPriorityQueue(size_type bufferSize);

    MPMCPriorityQueue(const MPMCPriorityQueue& ) = delete;
    MPMCPriorityQueue& operator =(const MPMCPriorityQueue& ) = delete;

    bool empty() const;

    bool Produce(int priority, value_type&& rdata);
    bool Consume(value_type *pdata);

private:
    UniquePtr<queue_type> _byPriority[_MaxPriority+1];
};
//----------------------------------------------------------------------------
template <typename _MPMCQueue, size_t _MaxPriority>
MPMCPriorityQueue<_MPMCQueue, _MaxPriority>::MPMCPriorityQueue() {
    forrange(i, 0, _MaxPriority+1)
        _byPriority[i].reset(new queue_type());
}
//----------------------------------------------------------------------------
template <typename _MPMCQueue, size_t _MaxPriority>
MPMCPriorityQueue<_MPMCQueue, _MaxPriority>::MPMCPriorityQueue(size_type bufferSize) {
    forrange(i, 0, _MaxPriority+1)
        _byPriority[i].reset(new queue_type(bufferSize));
}
//----------------------------------------------------------------------------
template <typename _MPMCQueue, size_t _MaxPriority>
bool MPMCPriorityQueue<_MPMCQueue, _MaxPriority>::empty() const {
    forrange(i, 0, _MaxPriority+1)
        if (not _byPriority[i]->empty())
            return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename _MPMCQueue, size_t _MaxPriority>
bool MPMCPriorityQueue<_MPMCQueue, _MaxPriority>::Produce(int priority, value_type&& rdata) {
    Assert(priority <= _MaxPriority);
    return _byPriority[size_t(priority)]->Produce(std::move(rdata));
}
//----------------------------------------------------------------------------
template <typename _MPMCQueue, size_t _MaxPriority>
bool MPMCPriorityQueue<_MPMCQueue, _MaxPriority>::Consume(value_type* pdata) {
    forrange(i, 0, _MaxPriority+1)
        if (_byPriority[i]->Consume(pdata))
            return true;

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
