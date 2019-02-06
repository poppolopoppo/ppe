#pragma once

#include "Core.h"

#include "Misc/Function.h"
#include "Thread/Fiber.h"

#include <mutex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
class FTaskFiberChunk; // .cpp only
class FTaskFiberPool : Meta::FNonCopyableNorMovable {
public:
    using FCallback = TFunction<void()>;

    struct FHandle;
    using FHandleRef = const FHandle*;

    struct CACHELINE_ALIGNED FHandle {
        size_t Index;
        mutable FFiber Fiber;
        mutable FCallback OnWakeUp;

        FTaskFiberChunk* Chunk() const {
            return (FTaskFiberChunk*)(this - Index);
        }

        void AttachWakeUpCallback(FCallback&& onWakeUp) const;
        void YieldFiber(FHandleRef to, bool release) const;
    };

    FTaskFiberPool(FCallback&& callback);
    ~FTaskFiberPool();

    const FCallback& Callback() const { return _callback; }

    FHandleRef AcquireFiber();
    void ReleaseFiber(FHandleRef handle);
    void YieldCurrentFiber(FHandleRef self, FHandleRef to, bool release);
    void StartThread();
    void ReleaseMemory();

    static FHandleRef CurrentHandleRef() {
        auto* h = (FHandleRef)FFiber::CurrentFiberData(); // @FHandle is passed down as each fiber data
        Assert(h);
        return h;
    }

private:
    const FCallback _callback;
    FTaskFiberChunk* _chunks;
    std::mutex _barrier;

    FTaskFiberChunk* AcquireChunk_();
};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
