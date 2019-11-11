#pragma once

#include "Core_fwd.h"

#include "Thread/Task_fwd.h"

#include "Container/Vector.h"
#include "Memory/MemoryView.h"
#include "Meta/PointerWFlags.h"
#include "Thread/AtomicSpinLock.h"

#include <atomic>

namespace PPE {
class FTaskFiberLocalCache;
class FTaskManagerImpl;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Holds a fiber with its original ITaskContext* and priority for resuming
//----------------------------------------------------------------------------
using FTaskFiberRef = const void*;
class FInteruptedTask {
public:
    FInteruptedTask() = default;

    FInteruptedTask(
        ITaskContext& ctx,
        FTaskFiberRef fiber,
        ETaskPriority priority ) NOEXCEPT
    :   _fiber(fiber) {
        _taskContextAndPriority.Reset(&ctx, uintptr_t(priority));
    }

    FTaskFiberRef Fiber() const { return _fiber; }
    ITaskContext* Context() const { return _taskContextAndPriority.Get(); }
    ETaskPriority Priority() const { return ETaskPriority(_taskContextAndPriority.Flag01()); }

    static PPE_CORE_API void Resume(const TMemoryView<FInteruptedTask>& tasks);

private:
    FTaskFiberRef _fiber;
    Meta::TPointerWFlags<ITaskContext> _taskContextAndPriority;
};
PPE_ASSUME_TYPE_AS_POD(FInteruptedTask)
//----------------------------------------------------------------------------
// This is the main synchronization API :
//  - use only when you need to wait for task(s) ;
//  - must be handled through ITaskContext ;
//  - value semantics, but not copyable nor movable ;
//  - see FAggregationPort bellow to wait for tasks already in-flight.
//----------------------------------------------------------------------------
class PPE_CORE_API FCompletionPort : Meta::FNonCopyableNorMovable {
public:
    FCompletionPort() = default;

#if USE_PPE_ASSERT
    ~FCompletionPort();
#endif

    bool Running() const { return (_countDown >= 0); }
    bool Finished() const { return (CP_Finished == _countDown); }

private: // only accessible through ITaskContext
    friend class FAggregationPort;
    friend class FTaskManagerImpl;

    void AttachCurrentFiber(FTaskFiberLocalCache& fibers, ETaskPriority priority) NOEXCEPT;
    void Start(size_t n) NOEXCEPT;
    void OnJobComplete();

private:
    enum state_t : int {
        CP_NotReady = -1,
        CP_Finished = -2,
    };
    std::atomic<int> _countDown{ CP_NotReady };

    FAtomicSpinLock _barrier;

    VECTORINSITU(Task, FInteruptedTask, 8) _queue;
    VECTORINSITU(Task, FCompletionPort*, 8) _children;

    static NO_INLINE void OnCountDownReachedZero_(FCompletionPort* port);
};
//----------------------------------------------------------------------------
// Can wait on multiple FCompletionPorts, *MUST* call Join() before destruction
//----------------------------------------------------------------------------
class PPE_CORE_API FAggregationPort {
public:
    FAggregationPort() NOEXCEPT;

    bool Running() const { return _port.Running(); }
    bool Finished() const { return _port.Finished(); }

    bool Attach(FCompletionPort* dep);
    bool Attach(FAggregationPort* other);

    void Join(ITaskContext& ctx);

private: // only accessible through ITaskContext
    friend class FTaskManagerImpl;

    FCompletionPort* Increment(size_t n) NOEXCEPT;

private:
    FCompletionPort _port;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
