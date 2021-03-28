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
};
//----------------------------------------------------------------------------
class PPE_RHI_API ICommandBatch : public FRefCountable {
public:
    virtual ~ICommandBatch() = default;

    virtual void TearDown() = 0;

    friend void OnStrongRefCountReachZero(ICommandBatch* batch) {
        Assert(batch);
        Assert_NoAssume(0 == batch->RefCount());
#if USE_PPE_SAFEPTR
        Assert_Lightweight(0 == batch->SafeRefCount());
#endif

        batch->TearDown();

        checked_delete(batch);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
