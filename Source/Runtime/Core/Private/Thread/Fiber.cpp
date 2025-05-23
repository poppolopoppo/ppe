﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Thread/Fiber.h"

#include "HAL/PlatformThread.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
enum : size_t {
    FiberStackReserveSize   = 1_MiB,
};
//----------------------------------------------------------------------------
static THREAD_LOCAL void* GCurrentThreadFiber = nullptr;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFiber::~FFiber() {
    Assert(nullptr == _pimpl); // need explicit Destroy() !
}
//----------------------------------------------------------------------------
FFiber::FFiber(FFiber&& rvalue) NOEXCEPT
    : _pimpl(rvalue._pimpl) {
    rvalue._pimpl = nullptr;
}
//----------------------------------------------------------------------------
FFiber& FFiber::operator =(FFiber&& rvalue) NOEXCEPT {
    Assert(nullptr == _pimpl);
    std::swap(_pimpl, rvalue._pimpl);

    return *this;
}
//----------------------------------------------------------------------------
void FFiber::Create(callback_t entryPoint, void *arg, size_t stackSize/* = 0 */) {
    Assert(!_pimpl);
    Assert(entryPoint);

    if (0 == stackSize)
        stackSize = FiberStackReserveSize;

    _pimpl = FPlatformThread::CreateFiber(stackSize, entryPoint, arg);

    Assert(_pimpl);

#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TRACKING_DATA(Fibers).AllocateSystem(stackSize);
#endif
}
//----------------------------------------------------------------------------
void FFiber::Resume() const {
    Assert(_pimpl);
    Assert(GCurrentThreadFiber);
    Assert(FPlatformThread::IsInFiber());
    Assert(FPlatformThread::CurrentFiber() != _pimpl);

    FPlatformThread::SwitchToFiber(static_cast<FPlatformThread::FFiber>(_pimpl));
}
//----------------------------------------------------------------------------
void FFiber::Destroy(size_t stackSize) {
    Assert(_pimpl);
    Assert(FPlatformThread::CurrentFiber() != _pimpl);

    FPlatformThread::DestroyFiber(static_cast<FPlatformThread::FFiber>(_pimpl));
    _pimpl = nullptr;

#if USE_PPE_MEMORYDOMAINS
    if (0 == stackSize)
        stackSize = FiberStackReserveSize;

    MEMORYDOMAIN_TRACKING_DATA(Fibers).DeallocateSystem(stackSize);
#else
    Unused(stackSize);
#endif
}
//----------------------------------------------------------------------------
void FFiber::Reset(void* pimpl /* = nullptr */) {
    _pimpl = pimpl;
}
//----------------------------------------------------------------------------
void FFiber::StackRegion(const void** pStackBottom, size_t* pStackSize) const NOEXCEPT {
    FPlatformThread::FiberStackRegion(static_cast<FPlatformThread::FFiber>(_pimpl), pStackBottom, pStackSize);
}
//----------------------------------------------------------------------------
void FFiber::Start() {
    Assert(!GCurrentThreadFiber);
    Assert(!FPlatformThread::IsInFiber());

    GCurrentThreadFiber = FPlatformThread::ConvertCurrentThreadToFiber();
}
//----------------------------------------------------------------------------
void FFiber::Shutdown() {
    Assert(GCurrentThreadFiber);
    Assert(FPlatformThread::IsInFiber());
    Assert(FPlatformThread::CurrentFiber() == GCurrentThreadFiber);

    FPlatformThread::RevertCurrentFiberToThread(static_cast<FPlatformThread::FFiber>(GCurrentThreadFiber));
    GCurrentThreadFiber = nullptr;
}
//----------------------------------------------------------------------------
void* FFiber::ThreadFiber() {
    Assert(GCurrentThreadFiber);
    Assert(FPlatformThread::IsInFiber());

    return GCurrentThreadFiber;
}
//----------------------------------------------------------------------------
void* FFiber::RunningFiber() {
    Assert(GCurrentThreadFiber);
    Assert(FPlatformThread::IsInFiber());

    return FPlatformThread::CurrentFiber();
}
//----------------------------------------------------------------------------
void* FFiber::RunningFiberIFP() {
    return (FPlatformThread::IsInFiber()
        ? FPlatformThread::CurrentFiber()
        : nullptr );
}
//----------------------------------------------------------------------------
bool FFiber::IsInFiber() {
    const bool result = (nullptr != GCurrentThreadFiber);
    Assert(result == !!FPlatformThread::IsInFiber());
    return result;
}
//----------------------------------------------------------------------------
void* FFiber::CurrentFiberData() {
    Assert(IsInFiber());

    return FPlatformThread::FiberData();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
