#pragma once

#include "Core_fwd.h"

#include "Container/Stack.h"
#include "Meta/Singleton.h"
#include "Meta/ThreadResource.h"
#include "Misc/Function.h"
#include "Thread/CriticalSection.h"
#include "Thread/Fiber.h"

#include <mutex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
class FTaskFiberPool : Meta::FNonCopyableNorMovable {
public:
    struct FHandle;
    friend struct FHandle;
    using FHandleRef = const FHandle*;

    using FCallback = void(*)();

    explicit FTaskFiberPool(FCallback&& callback) NOEXCEPT;
    ~FTaskFiberPool();

    FCallback Callback() const { return _callback; }

    FHandleRef AcquireFiber();
    bool OwnsFiber(FHandleRef handle) const NOEXCEPT;
    void ReleaseFiber(FHandleRef handle);
    void YieldFiber(FHandleRef self, FHandleRef to, bool release);

    void StartThread();
    NORETURN void ShutdownThread();

    void ReleaseMemory();

#if !USE_PPE_FINAL_RELEASE
    void UsageStats(size_t* reserved, size_t* inUse) NOEXCEPT;
#endif

    static void AttachWakeUpCallback(FHandleRef fiber, TFunction<void()>&& onWakeUp);
    static size_t ReservedStackSize() NOEXCEPT;
    static void ResetWakeUpCallback(FHandleRef fiber);
    static void YieldCurrentFiber(FHandleRef to, bool release);

    static FHandleRef CurrentHandleRef() {
        auto* h = static_cast<FHandleRef>(FFiber::CurrentFiberData()); // @FHandle is passed down as each fiber data
        Assert(h);
        return h;
    }

private:
    const FCallback _callback;
    std::atomic<FHandleRef> _freeFibers;

#if !USE_PPE_FINAL_RELEASE
    std::atomic<int> _numFibersAvailable;
    std::atomic<int> _numFibersReserved;
#endif
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
    FHandleRef _lastFreeFiber{ nullptr };
};
//----------------------------------------------------------------------------
class PPE_CORE_API FGlobalFiberPool : private Meta::TSingleton<FTaskFiberPool, FGlobalFiberPool> {
    friend class Meta::TSingleton<FTaskFiberPool, FGlobalFiberPool>;
    using singleton_type = Meta::TSingleton<FTaskFiberPool, FGlobalFiberPool>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;

public:
    using FCallback = FTaskFiberPool::FCallback;

    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create(FCallback&& entryPoint);
    static void Destroy();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
