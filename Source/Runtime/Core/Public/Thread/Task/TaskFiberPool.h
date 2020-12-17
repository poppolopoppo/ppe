#pragma once

#include "Core_fwd.h"

#include "Container/Stack.h"
#include "Meta/ThreadResource.h"
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
        mutable FFiber Fiber;
        mutable FCallback OnWakeUp;
        u32 Index;

        FTaskFiberChunk* Chunk() const {
            return (FTaskFiberChunk*)(this - Index);
        }

        void AttachWakeUpCallback(FCallback&& onWakeUp) const;
        void YieldFiber(FHandleRef to, bool release) const;
    };

    explicit FTaskFiberPool(FCallback&& callback) NOEXCEPT;
    ~FTaskFiberPool();

    const FCallback& Callback() const { return _callback; }

    FHandleRef AcquireFiber();
    bool OwnsFiber(FHandleRef handle) const NOEXCEPT;
    void ReleaseFiber(FHandleRef handle);
    void YieldCurrentFiber(FHandleRef self, FHandleRef to, bool release);
    void StartThread();
    void ReleaseMemory();

#if !USE_PPE_FINAL_RELEASE
    NO_INLINE void UsageStats(size_t* reserved, size_t* inUse);
#endif

    static size_t ReservedStackSize();
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
class FTaskFiberLocalCache
    : Meta::FNonCopyableNorMovable
    , Meta::FThreadResource {
public:
    explicit FTaskFiberLocalCache(FTaskFiberPool& pool) NOEXCEPT;
    ~FTaskFiberLocalCache();

    using FHandleRef = FTaskFiberPool::FHandleRef;

    FHandleRef AcquireFiber();
    void ReleaseFiber(FHandleRef handle);

    NO_INLINE void ReleaseMemory();

private:
    FTaskFiberPool& _pool;
    TFixedSizeStack<FHandleRef, 3> _freed;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
