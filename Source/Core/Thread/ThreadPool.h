#pragma once

#include "Core/Core.h"

#include "Core/Meta/Singleton.h"
#include "Core/Thread/Task/TaskCompletionPort.h"
#include "Core/Thread/Task/TaskEvaluator.h"

#include <chrono>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadPool {
public:
    ThreadPool(const char *name, size_t workerCount);
    ~ThreadPool();

    TaskEvaluator *Evaluator() { return &_evaluator; }
    const TaskEvaluator *Evaluator() const { return &_evaluator; }

    TaskCompletionPort *CompletionPort() { return &_completionPort; }
    const TaskCompletionPort *CompletionPort() const { return &_completionPort; }

    void Produce(ITask *task);

    bool WaitOne();
    bool WaitOne(const std::chrono::microseconds& timeout);
    void WaitAll();

private:
    TaskEvaluator _evaluator;
    TaskCompletionPort _completionPort;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GlobalThreadPool : Meta::Singleton<ThreadPool, GlobalThreadPool> {
public:
    typedef Meta::Singleton<ThreadPool, GlobalThreadPool> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create();
};
//----------------------------------------------------------------------------
class IOThreadPool : Meta::Singleton<ThreadPool, IOThreadPool> {
public:
    typedef Meta::Singleton<ThreadPool, IOThreadPool> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadPoolStartup {
public:
    static void Start();
    static void Shutdown();

    static void Clear();

    ThreadPoolStartup() { Start(); }
    ~ThreadPoolStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
