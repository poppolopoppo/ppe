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
    PCommandBuffer Buffer;
    PCommandBatch Batch;

    FCommandBufferBatch() = default;
    FCommandBufferBatch(std::nullptr_t) NOEXCEPT {}

    FCommandBufferBatch(PCommandBuffer&& rbuffer, PCommandBatch&& rbatch) NOEXCEPT
    :   Buffer(std::move(rbuffer))
    ,   Batch(std::move(rbatch))
    {}

    FCommandBufferBatch(const FCommandBufferBatch&) = default;
    FCommandBufferBatch& operator =(const FCommandBufferBatch&) = default;

    FCommandBufferBatch(FCommandBufferBatch&&) = default;
    FCommandBufferBatch& operator =(FCommandBufferBatch&&) = default;

#if USE_PPE_RHIDEBUG
    ~FCommandBufferBatch() {
        AssertMessage(L"command buffer batch was not executed", not Buffer);
    }
#endif

    bool Valid() const { return (!!Buffer); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    ICommandBuffer* operator ->() const NOEXCEPT {
        Assert(Buffer);
        return Buffer.get();
    }
};
//----------------------------------------------------------------------------
class PPE_RHI_API ICommandBatch : public FRefCountable {
public:
    virtual ~ICommandBatch() = default;

    virtual void ReleaseForRecycling() NOEXCEPT = 0;

    // call TearDown() when released, just before delete
    friend void OnStrongRefCountReachZero(ICommandBatch* batch) {
        Assert(batch);
        Assert_NoAssume(0 == batch->RefCount());
#if USE_PPE_SAFEPTR
        Assert_Lightweight(0 == batch->SafeRefCount());
#endif

        batch->ReleaseForRecycling();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
