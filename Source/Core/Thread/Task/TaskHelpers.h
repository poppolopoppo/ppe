#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Thread/Task/Task.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Result>
class TaskFuture : public Task {
public:
    typedef std::function<_Result()> function_type;

    TaskFuture(function_type&& func);
    ~TaskFuture();

    bool Available() const { return _available; }
    const _Result& Result() const;// will wait for the result if not available

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual void Run(ITaskContext& ctx) override;

private:
    const function_type _func;
    _Result _result;

    char cacheline_pad_t[CACHELINE_SIZE];// no thread collisions on _available :
    std::atomic<bool> _available;
};
//----------------------------------------------------------------------------
template <typename _Result>
using PFuture = RefPtr< const TaskFuture<_Result> >;
//----------------------------------------------------------------------------
template <typename _Lambda>
TaskFuture< decltype(std::declval<_Lambda>()()) >*
    Future(TaskManager& manager, _Lambda&& func, TaskPriority priority = TaskPriority::Normal);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskProcedure : public Task {
public:
    typedef std::function<void(ITaskContext& ctx)> function_type;

    TaskProcedure(function_type&& func);

    TaskProcedure(const TaskProcedure& ) = delete;
    TaskProcedure& operator=(const TaskProcedure& ) = delete;

    SINGLETON_POOL_ALLOCATED_DECL();

protected:
    virtual void Run(ITaskContext& ctx) override;

private:
    function_type _func;
};
//----------------------------------------------------------------------------
void ASync(TaskManager& manager, std::function<void()>&& fireAndForget, TaskPriority priority = TaskPriority::Normal);
//----------------------------------------------------------------------------
void ASync(TaskManager& manager, std::function<void(ITaskContext&)>&& fireAndForget, TaskPriority priority = TaskPriority::Normal);
//----------------------------------------------------------------------------
template <typename _It, typename _Lambda>
void ParallelForRange(
    TaskManager& manager,
    _It first, _It last, _Lambda&& lambda,
    TaskPriority priority = TaskPriority::Normal );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define async(...) \
    Core::ASync(Core::GlobalThreadPool::Instance(), __VA_ARGS__)
//----------------------------------------------------------------------------
#define future(...) \
    Core::Future(Core::GlobalThreadPool::Instance(), __VA_ARGS__)
//----------------------------------------------------------------------------
#define parallel_forrange(_First, _Last, ...) \
    Core::ParallelForRange(Core::GlobalThreadPool::Instance(), _First, _Last, __VA_ARGS__)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "TaskHelpers-inl.h"
