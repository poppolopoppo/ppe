#include "stdafx.h"

#include "Fiber.h"

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
    FiberStackCommitSize    = 0,
    FiberStackReserveSize   = (2048/* kb */<<10),
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
FFiber::FFiber(FFiber&& rvalue)
    : _pimpl(rvalue._pimpl) {
    rvalue._pimpl = nullptr;
}
//----------------------------------------------------------------------------
FFiber& FFiber::operator =(FFiber&& rvalue) {
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

    _pimpl = FPlatformThread::CreateFiber(
        FiberStackCommitSize,
        stackSize,
        entryPoint,
        arg );

    Assert(_pimpl);

#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TRACKING_DATA(Fibers).Allocate(1, stackSize);
#endif
}
//----------------------------------------------------------------------------
void FFiber::Resume() {
    Assert(_pimpl);
    Assert(GCurrentThreadFiber);
    Assert(FPlatformThread::IsInFiber());
    Assert(FPlatformThread::CurrentFiber() != _pimpl);

    FPlatformThread::SwitchToFiber(_pimpl);
}
//----------------------------------------------------------------------------
void FFiber::Destroy(size_t stackSize) {
    Assert(_pimpl);
    Assert(FPlatformThread::CurrentFiber() != _pimpl);

    FPlatformThread::DestroyFiber(_pimpl);
    _pimpl = nullptr;

#if USE_PPE_MEMORYDOMAINS
    if (0 == stackSize)
        stackSize = FiberStackReserveSize;

    MEMORYDOMAIN_TRACKING_DATA(Fibers).Deallocate(1, stackSize);
#else
    UNUSED(stackSize);
#endif
}
//----------------------------------------------------------------------------
void FFiber::Reset(void* pimpl /* = nullptr */) {
    _pimpl = pimpl;
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

    FPlatformThread::RevertCurrentFiberToThread(GCurrentThreadFiber);
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
    Assert(result == (TRUE == FPlatformThread::IsInFiber()) );
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
