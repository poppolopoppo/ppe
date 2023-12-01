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

    FCommandBufferBatch(const FCommandBufferBatch&) = delete;
    FCommandBufferBatch& operator =(const FCommandBufferBatch&) = delete;

    FCommandBufferBatch(FCommandBufferBatch&&) = default;
    FCommandBufferBatch& operator =(FCommandBufferBatch&&) = default;

#if USE_PPE_RHIDEBUG
    ~FCommandBufferBatch() {
        AssertMessage("command buffer was not executed", not _buffer);
    }
#endif

    const PCommandBuffer& Buffer() const { return _buffer; }
    const PCommandBatch& Batch() const { return _batch; }

    operator const PCommandBuffer& () const NOEXCEPT { return _buffer; }
    operator const PCommandBatch& () const NOEXCEPT { return _batch; }

    operator SCommandBuffer () const NOEXCEPT { return _buffer; }
    operator SCommandBatch () const NOEXCEPT { return _batch; }

    bool Valid() const { return (!!_buffer); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

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
