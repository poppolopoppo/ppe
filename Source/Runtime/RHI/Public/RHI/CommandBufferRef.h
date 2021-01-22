#pragma once

#include "RHI_fwd.h"

#include "RHI/CommandBuffer.h"

#include <atomic>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ICommandBatch {
public:
    friend struct FCommandBufferRef;

    std::atomic<int> Counter;

    virtual ~ICommandBatch() = default;
    virtual void Release() = 0;
};
//----------------------------------------------------------------------------
struct FCommandBufferRef {
public:
    FCommandBufferRef() = default;
    FCommandBufferRef(std::nullptr_t) {}

    FCommandBufferRef(SCommandBuffer&& rbuffer, ICommandBatch* batch)
    :   _buffer(std::move(rbuffer))
    ,   _batch(batch) {
        IncRef_();
    }

    FCommandBufferRef(const FCommandBufferRef& other)
    :   _buffer(other._buffer)
    ,   _batch(other._batch) {
        IncRef_();
    }

    FCommandBufferRef(FCommandBufferRef&& rvalue)
    :   _buffer(std::move(rvalue._buffer))
    ,   _batch(rvalue._batch) {
        rvalue._batch = nullptr;
    }

    FCommandBufferRef& operator =(const FCommandBufferRef& other) {
        DecRef_();
        _buffer = other._buffer;
        _batch = other._batch;
        IncRef_();
        return (*this);
    }

    FCommandBufferRef& operator =(FCommandBufferRef&& rvalue) {
        _buffer = std::move(rvalue._buffer);
        _batch = rvalue._batch;
        rvalue._batch = nullptr;
        return (*this);
    }

    FCommandBufferRef& operator =(std::nullptr_t) {
        DecRef_();
        _buffer = nullptr;
        _batch = nullptr;
        return (*this);
    }

    ~FCommandBufferRef() {
        DecRef_();
    }

    SCommandBuffer Buffer() const { return _buffer; }
    ICommandBatch* Batch() const { return _batch; }

    ICommandBuffer* operator ->() const { Assert(_buffer); return _buffer.get(); }

private:
    SCommandBuffer _buffer{ nullptr };
    ICommandBatch* _batch{ nullptr };

    void IncRef_() {
        if (_batch)
            _batch->Counter.fetch_add(1, std::memory_order_relaxed);
    }
    void DecRef_() {
        if (_batch) {
            const int old = _batch->Counter.fetch_sub(1, std::memory_order_relaxed);
            Assert(old > 0);
            if (1 == old) {
                std::atomic_thread_fence( std::memory_order_acquire );
                _batch->Release();
            }
        }
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
