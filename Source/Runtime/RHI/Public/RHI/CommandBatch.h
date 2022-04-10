#pragma once

#include "RHI_fwd.h"

#include "RHI/CommandBuffer.h"

#include <atomic>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FCommandBufferBatch {
public:
    FCommandBufferBatch() = default;
    FCommandBufferBatch(std::nullptr_t) NOEXCEPT {}

    FCommandBufferBatch(PCommandBuffer&& rbuffer, PCommandBatch&& rbatch) NOEXCEPT
    :   _buffer(std::move(rbuffer))
    ,   _batch(std::move(rbatch))
    {}

    FCommandBufferBatch(const FCommandBufferBatch&) = default;
    FCommandBufferBatch& operator =(const FCommandBufferBatch&) = default;

    FCommandBufferBatch(FCommandBufferBatch&&) = default;
    FCommandBufferBatch& operator =(FCommandBufferBatch&&) = default;

#if USE_PPE_RHIDEBUG
    ~FCommandBufferBatch() {
        AssertMessage(L"command buffer was not executed", not _buffer);
    }
#endif

    bool Valid() const { return (!!_buffer); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    const PCommandBuffer& Buffer() const { return _buffer; }
    const PCommandBatch& Batch() const { return _batch; }

    ICommandBuffer* operator ->() const NOEXCEPT {
        Assert(_buffer);
        return _buffer.get();
    }

private:
    PCommandBuffer _buffer;
    PCommandBatch _batch;
};
//----------------------------------------------------------------------------
class PPE_RHI_API ICommandBatch : public FRefCountable {
public:
    virtual ~ICommandBatch() = default;

    // Implement for refptr recycling, see RefPtr-inl.h
    virtual void OnStrongRefCountReachZero() NOEXCEPT = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
