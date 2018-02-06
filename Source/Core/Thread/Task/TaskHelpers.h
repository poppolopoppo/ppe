#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Meta/Function.h"
#include "Core/Thread/Task/Task.h"

namespace Core {
class FTaskManager;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFireAndForget {
public:
    virtual ~FFireAndForget() {}

    void RunAndSuicide(ITaskContext& ctx) {
        Run(ctx);
        checked_delete(this); // WILL BE DELETED HERE !
    }

protected:
    virtual void Run(ITaskContext& ctx) = 0;
};
//----------------------------------------------------------------------------
CORE_API void FireAndForget(
    FFireAndForget* task/* will be deleted in worker thread after Run() */,
    ETaskPriority priority = ETaskPriority::Normal,
    FTaskManager* manager = nullptr/* uses FGlobalThreadPool by default */);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TFuture : public FRefCountable {
public:
    typedef Meta::TFunction<T()> func_type;

    explicit TFuture(func_type&& func);
    ~TFuture();

    TFuture(const TFuture& ) = delete;
    TFuture& operator =(const TFuture&) = delete;

    bool Available() const { return (Ready == _state); }

    T& Result(); // blocking
    T* ResultIFP(); // non blocking

    void Async(ETaskPriority priority, FTaskManager* manager);

    SINGLETON_POOL_ALLOCATED_DECL();

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
CORE_API void Async(
    FTaskFunc&& task,
    ETaskPriority priority = ETaskPriority::Normal,
    FTaskManager* manager = nullptr/* uses FGlobalThreadPool by default */);
//----------------------------------------------------------------------------
template <typename T>
PFuture<T> Future(
    Meta::TFunction<T()>&& func,
    ETaskPriority priority = ETaskPriority::Normal,
    FTaskManager* manager = nullptr/* uses FGlobalThreadPool by default */);
//----------------------------------------------------------------------------
template <typename _It>
void ParallelFor(
    _It first, _It last,
    const Meta::TFunction<void(_It)>& foreach,
    ETaskPriority priority = ETaskPriority::Normal,
    FTaskManager* manager = nullptr/* uses FGlobalThreadPool by default */);
//----------------------------------------------------------------------------
template <typename _It>
void ParallelFor(
    _It first, _It last,
    const Meta::TFunction<void(decltype(*std::declval<_It>()))>& foreach_item,
    ETaskPriority priority = ETaskPriority::Normal,
    FTaskManager* manager = nullptr/* uses FGlobalThreadPool by default */);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "TaskHelpers-inl.h"
