#include "stdafx.h"

#include "ThreadPool.h"

#include "TaskWorker.h"

#include <thread>

#define CORE_THREAPOOL_CAPACITY 1024

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ThreadPool::ThreadPool(const char *name, size_t workerCount)
:   _evaluator(name, workerCount, CORE_THREAPOOL_CAPACITY)
,   _completionPort("ThreadPool", &_evaluator) {
    _evaluator.Start();
}
//----------------------------------------------------------------------------
ThreadPool::~ThreadPool() {
    _completionPort.WaitAll();
    _evaluator.Shutdown();
}
//----------------------------------------------------------------------------
void ThreadPool::Produce(ITask *task) {
    _completionPort.Produce(task);
}
//----------------------------------------------------------------------------
bool ThreadPool::WaitOne() {
    return _completionPort.WaitOne();
}
//----------------------------------------------------------------------------
bool ThreadPool::WaitOne(const std::chrono::microseconds& timeout) {
    return _completionPort.WaitOne(timeout);
}
//----------------------------------------------------------------------------
void ThreadPool::WaitAll() {
    _completionPort.WaitAll();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void GlobalThreadPool::Create() {
    parent_type::Create("GlobalThreadPool", std::thread::hardware_concurrency() /* one per logical core */);
}
//----------------------------------------------------------------------------
void IOThreadPool::Create() {
    parent_type::Create("IOThreadPool", 1 /* IO should be operated in 1 thread to prevent slow seek */);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ThreadPoolStartup::Start() {
    GlobalThreadPool::Create();
    IOThreadPool::Create();
}
//----------------------------------------------------------------------------
void ThreadPoolStartup::Shutdown() {
    IOThreadPool::Destroy();
    GlobalThreadPool::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
