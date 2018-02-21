#include "stdafx.h"

#include "Fiber.h"

#include "Memory/MemoryDomain.h"

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#else
#   error "OS not yet supported"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
enum : size_t {
    FiberStackCommitSize    = 0,
    FiberStackReserveSize   = (2048/* kb */<<10),
    FiberFlags              = FIBER_FLAG_FLOAT_SWITCH,
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

    _pimpl = ::CreateFiberEx(
        FiberStackCommitSize,
        stackSize,
        FiberFlags,
        entryPoint,
        arg );

    Assert(_pimpl);

#ifdef USE_MEMORY_DOMAINS
    MEMORY_DOMAIN_TRACKING_DATA(Fibers).Allocate(1, stackSize);
    MEMORY_DOMAIN_TRACKING_DATA(Reserved).Allocate(1, stackSize); // can't be logged since we don't control the allocation
#endif
}
//----------------------------------------------------------------------------
void FFiber::Resume() {
    Assert(_pimpl);
    Assert(GCurrentThreadFiber);
    Assert(::IsThreadAFiber());
    Assert(::GetCurrentFiber() != _pimpl);

    ::SwitchToFiber(_pimpl);
}
//----------------------------------------------------------------------------
void FFiber::Destroy(size_t stackSize) {
    Assert(_pimpl);
    Assert(::GetCurrentFiber() != _pimpl);

    ::DeleteFiber(_pimpl);
    _pimpl = nullptr;

#ifdef USE_MEMORY_DOMAINS
    if (0 == stackSize)
        stackSize = FiberStackReserveSize;

    MEMORY_DOMAIN_TRACKING_DATA(Fibers).Deallocate(1, stackSize);
    MEMORY_DOMAIN_TRACKING_DATA(Reserved).Deallocate(1, stackSize); // can't be logged since we don't control the allocation
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
    Assert(!::IsThreadAFiber());

    GCurrentThreadFiber = ::ConvertThreadToFiberEx(nullptr, FiberFlags);
}
//----------------------------------------------------------------------------
void FFiber::Shutdown() {
    Assert(GCurrentThreadFiber);
    Assert(::IsThreadAFiber());
    Assert(::GetCurrentFiber() == GCurrentThreadFiber);

    ::ConvertFiberToThread();
    GCurrentThreadFiber = nullptr;
}
//----------------------------------------------------------------------------
void* FFiber::ThreadFiber() {
    Assert(GCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    return GCurrentThreadFiber;
}
//----------------------------------------------------------------------------
void* FFiber::RunningFiber() {
    Assert(GCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    return ::GetCurrentFiber();
}
//----------------------------------------------------------------------------
void* FFiber::RunningFiberIFP() {
    return ::IsThreadAFiber()
        ? ::GetCurrentFiber()
        : nullptr;
}
//----------------------------------------------------------------------------
bool FFiber::IsInFiber() {
    const bool result = (nullptr != GCurrentThreadFiber);
    Assert(result == (TRUE == ::IsThreadAFiber()) );
    return result;
}
//----------------------------------------------------------------------------
void* FFiber::CurrentFiberData() {
    Assert(IsInFiber());
    return ::GetFiberData();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
