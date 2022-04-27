#pragma once

#include "Core.h"

#include "HAL/PlatformThread.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSynchronizationBarrier : Meta::FNonCopyableNorMovable {
public:
    explicit FSynchronizationBarrier(size_t numThreads) { FPlatformThread::CreateSynchronizationBarrier(&_barrier, numThreads); }
    ~FSynchronizationBarrier() { FPlatformThread::DestroySynchronizationBarrier(&_barrier); }

    FSynchronizationBarrier(const FSynchronizationBarrier&) = delete;
    FSynchronizationBarrier& operator =(const FSynchronizationBarrier&) = delete;

    FSynchronizationBarrier(FSynchronizationBarrier&& rvalue) = delete;
    FSynchronizationBarrier& operator =(FSynchronizationBarrier&& rvalue) = delete;

    void Wait() const NOEXCEPT { FPlatformThread::EnterSynchronizationBarrier(_barrier); }

private:
    mutable FPlatformThread::FSynchronizationBarrier _barrier{};
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
