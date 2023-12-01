#pragma once

#include "Core_fwd.h"

#include "Thread/Task/Task.h"
#include "Thread/Task/TaskManager.h"

#include "Misc/Function.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FFireAndForget {
public:
    virtual ~FFireAndForget() = default;

    void RunAndSuicide(ITaskContext& ctx) {
        Run(ctx);
        checked_delete(this); // WILL BE DELETED HERE !
    }

protected:
    virtual void Run(ITaskContext& ctx) = 0;
};
//----------------------------------------------------------------------------
PPE_CORE_API void FireAndForget(
    FFireAndForget* task/* will be deleted in worker thread after Run() */,
    ETaskPriority priority = ETaskPriority::Normal,
    ITaskContext* context = nullptr/* uses FGlobalThreadPool by default */);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TFuture : public FRefCountable {
public:
    typedef TFunction<T()> func_type;

    explicit TFuture(func_type&& func);
    ~TFuture();

    TFuture(const TFuture& ) = delete;
    TFuture& operator =(const TFuture&) = delete;

    NODISCARD bool Available() const { return (Ready == _state); }

    NODISCARD T& Result(); // blocking
    NODISCARD T* ResultIFP(); // non blocking

    void Async(ETaskPriority priority, ITaskContext* context);

private:
    enum EState_ : int {
        Idle = 0,
        Pending,
        Ready,
    };

    std::atomic<int> _state;
    func_type _func;
    T _value;
};
//----------------------------------------------------------------------------
template <typename T>
using PFuture = TRefPtr< TFuture<T> >;
template <typename T>
using SFuture = TSafePtr< TFuture<T> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API void Async(
    FTaskFunc&& task,
    ETaskPriority priority = ETaskPriority::Normal,
    ITaskContext* context = nullptr/* uses FGlobalThreadPool by default */);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD PFuture<T> Future(
    TFunction<T()>&& func,
    ETaskPriority priority = ETaskPriority::Normal,
    ITaskContext* context = nullptr/* uses FGlobalThreadPool by default */);
//----------------------------------------------------------------------------
PPE_CORE_API void ParallelFor(
    size_t first, size_t last,
    const TFunction<void(size_t)>& foreach,
    ETaskPriority priority = ETaskPriority::Normal,
    ITaskContext* context = nullptr/* uses FHighPriorityThreadPool by default */);
//----------------------------------------------------------------------------
template <typename _It>
void ParallelForEach(
    _It first, _It last,
    const TFunction<void(_It)>& foreach_it,
    ETaskPriority priority = ETaskPriority::Normal,
    ITaskContext* context = nullptr/* uses FHighPriorityThreadPool by default */);
//----------------------------------------------------------------------------
template <typename _It>
void ParallelForEachValue(
    _It first, _It last,
    const TFunction<void(typename Meta::TIteratorTraits<_It>::value_type)>& foreach_value,
    ETaskPriority priority = ETaskPriority::Normal,
    ITaskContext* context = nullptr/* uses FHighPriorityThreadPool by default */);
//----------------------------------------------------------------------------
template <typename _It>
void ParallelForEachRef(
    _It first, _It last,
    const TFunction<void(typename Meta::TIteratorTraits<_It>::reference)>& foreach_ref,
    ETaskPriority priority = ETaskPriority::Normal,
    ITaskContext* context = nullptr/* uses FHighPriorityThreadPool by default */);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API int ParallelSum(
    size_t first, size_t last,
    const TFunction<int(size_t)>& sum,
    ETaskPriority priority = ETaskPriority::Normal,
    ITaskContext* context = nullptr/* uses FHighPriorityThreadPool by default */);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "TaskHelpers-inl.h"
