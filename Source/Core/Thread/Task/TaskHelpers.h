#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Meta/Function.h"
#include "Core/Thread/Task/Task.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Result>
class TTaskFuture : public FTask {
public:
    typedef Meta::TFunction<_Result()> function_type;

    TTaskFuture(function_type&& func);
    virtual ~TTaskFuture();

    bool Available() const { return _available; }
    const _Result& Result() const;// will wait for the result if not available

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual void Run(ITaskContext& ctx) override final;

private:
    const function_type _func;
    std::atomic<bool> _available;

    char cacheline_pad_t[CACHELINE_SIZE];// no thread collisions on _available :
    _Result _result;
};
//----------------------------------------------------------------------------
template <typename _Lambda>
TTaskFuture< decltype(std::declval<_Lambda>()()) >*
    MakeFuture(_Lambda&& func);
//----------------------------------------------------------------------------
template <typename _Result>
using PFuture = TRefPtr< const TTaskFuture<_Result> >;
//----------------------------------------------------------------------------
template <typename _Lambda>
TTaskFuture< decltype(std::declval<_Lambda>()()) >*
    Future(FTaskManager& manager, _Lambda&& func, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTaskProcedure : public FTask {
public:
    typedef Meta::TFunction<void(ITaskContext& ctx)> function_type;

    FTaskProcedure(function_type&& func);

    FTaskProcedure(const FTaskProcedure& ) = delete;
    FTaskProcedure& operator=(const FTaskProcedure& ) = delete;

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual void Run(ITaskContext& ctx) override final;

private:
    function_type _func;
};
//----------------------------------------------------------------------------
FTaskProcedure* MakeAsync(Meta::TFunction<void()>&& fireAndForget);
FTaskProcedure* MakeAsync(Meta::TFunction<void(ITaskContext&)>&& fireAndForget);
//----------------------------------------------------------------------------
void ASync(FTaskManager& manager, Meta::TFunction<void()>&& fireAndForget, ETaskPriority priority = ETaskPriority::Normal);
void ASync(FTaskManager& manager, Meta::TFunction<void(ITaskContext&)>&& fireAndForget, ETaskPriority priority = ETaskPriority::Normal);
//----------------------------------------------------------------------------
template <typename _It, typename _Lambda>
void ParallelForRange(
    FTaskManager& manager,
    _It first, _It last, _Lambda&& lambda,
    ETaskPriority priority = ETaskPriority::Normal );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define async(...) \
    Core::ASync(Core::FGlobalThreadPool::Instance(), __VA_ARGS__)
//----------------------------------------------------------------------------
#define future(...) \
    Core::Future(Core::FGlobalThreadPool::Instance(), __VA_ARGS__)
//----------------------------------------------------------------------------
#define parallel_for(_First, _Last, ...) \
    Core::ParallelForRange(Core::FGlobalThreadPool::Instance(), _First, _Last, __VA_ARGS__)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "TaskHelpers-inl.h"
