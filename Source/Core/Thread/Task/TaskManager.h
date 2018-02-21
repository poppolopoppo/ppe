#pragma once

#include "Core/Core.h"

#include "Core/Thread/Task/Task.h"

#include "Core/IO/StringView.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
enum class EThreadPriority;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(TaskCounter);
class FTaskManager;
class FTaskManagerImpl;
//----------------------------------------------------------------------------
CORE_API ITaskContext& CurrentTaskContext();
//----------------------------------------------------------------------------
class CORE_API FTaskWaitHandle {
public:
    friend class FTaskManagerImpl;

    FTaskWaitHandle();
    FTaskWaitHandle(ETaskPriority priority, PTaskCounter&& counter);
    ~FTaskWaitHandle();

    FTaskWaitHandle(const FTaskWaitHandle& other) = delete;
    FTaskWaitHandle& operator =(const FTaskWaitHandle& other) = delete;

    FTaskWaitHandle(FTaskWaitHandle&& rvalue);
    FTaskWaitHandle& operator =(FTaskWaitHandle&& rvalue);

    ETaskPriority Priority() const { return _priority; }
    const FTaskCounter* Counter() const { return _counter.get(); }

    bool Valid() const { return _counter.valid(); }
    bool Finished() const;

private:
    ETaskPriority _priority;
    PTaskCounter _counter;
};
//----------------------------------------------------------------------------
class ITaskContext {
public:
    virtual ~ITaskContext() {}

    virtual void Run(FTaskWaitHandle* phandle, FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void Run(FTaskWaitHandle* phandle, const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) = 0;
    virtual void WaitFor(FTaskWaitHandle& handle, ITaskContext* resume = nullptr) = 0;
    virtual void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal, ITaskContext* resume = nullptr) = 0;

    void RunOne(FTaskWaitHandle* phandle, const FTaskFunc& task, ETaskPriority priority) {
        Run(phandle, MakeView(&task, &task+1), priority);
    }
};
//----------------------------------------------------------------------------
class CORE_API FTaskManager {
public:
    FTaskManager(const FStringView& name, size_t threadTag, size_t workerCount, EThreadPriority priority);
    ~FTaskManager();

    FTaskManager(const FTaskManager& ) = delete;
    FTaskManager& operator =(const FTaskManager& ) = delete;

    const FStringView& Name() const { return _name; }
    size_t ThreadTag() const { return _threadTag; }
    size_t WorkerCount() const { return _workerCount; }
    EThreadPriority Priority() const { return _priority; }

    void Start(const TMemoryView<const size_t>& threadAffinities);
    void Shutdown();

    ITaskContext* Context() const;

    void Run(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, ETaskPriority priority = ETaskPriority::Normal) const;
    void RunAndWaitFor(const TMemoryView<const FTaskFunc>& tasks, const FTaskFunc& whileWaiting, ETaskPriority priority = ETaskPriority::Normal) const;

    void Run(FTaskFunc&& rtask, ETaskPriority priority = ETaskPriority::Normal) const;
    void Run(const FTaskFunc& task, ETaskPriority priority = ETaskPriority::Normal) const {
        Run(MakeView(&task, &task+1), priority);
    }

    void RunAndWaitFor(const FTaskFunc& task, ETaskPriority priority = ETaskPriority::Normal) const {
        RunAndWaitFor(MakeView(&task, &task+1), priority);
    }

    void WaitForAll() const;

    FTaskManagerImpl* Pimpl() const { return _pimpl.get(); }

private:
    TUniquePtr<FTaskManagerImpl> _pimpl;

    const FStringView _name;
    const size_t _threadTag;
    const size_t _workerCount;
    const EThreadPriority _priority;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
