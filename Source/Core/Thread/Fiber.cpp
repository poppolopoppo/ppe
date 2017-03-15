#include "stdafx.h"

#include "Fiber.h"

#ifdef PLATFORM_WINDOWS
#   include <windows.h>
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
static THREAD_LOCAL void *gCurrentThreadFiber = nullptr;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFiber::~FFiber() {
    if (_pimpl)
        Destroy();
}
//----------------------------------------------------------------------------
FFiber::FFiber(FFiber&& rvalue)
    : _pimpl(rvalue._pimpl) {
    rvalue._pimpl = nullptr;
}
//----------------------------------------------------------------------------
FFiber& FFiber::operator =(FFiber&& rvalue) {
    if (_pimpl)
        Destroy();

    Assert(nullptr == _pimpl);
    std::swap(_pimpl, rvalue._pimpl);

    return *this;
}
//----------------------------------------------------------------------------
void FFiber::Create(callback_t entryPoint, void *arg, size_t stackSize/* = 0 */) {
    Assert(!_pimpl);
    Assert(entryPoint);
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    if (0 == stackSize)
        stackSize = FiberStackReserveSize;

    _pimpl = ::CreateFiberEx(
        FiberStackCommitSize,
        stackSize,
        FiberFlags,
        entryPoint,
        arg );

    Assert(_pimpl);
}
//----------------------------------------------------------------------------
void FFiber::Resume() {
    Assert(_pimpl);
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());
    Assert(::GetCurrentFiber() != _pimpl);

    ::SwitchToFiber(_pimpl);
}
//----------------------------------------------------------------------------
void FFiber::Destroy() {
    Assert(_pimpl);
    Assert(::GetCurrentFiber() != _pimpl);

    ::DeleteFiber(_pimpl);
    _pimpl = nullptr;
}
//----------------------------------------------------------------------------
void FFiber::Reset(void* pimpl /* = nullptr */) {
    _pimpl = pimpl;
}
//----------------------------------------------------------------------------
void FFiber::Start() {
    Assert(!gCurrentThreadFiber);
    Assert(!::IsThreadAFiber());

    gCurrentThreadFiber = ::ConvertThreadToFiberEx(nullptr, FiberFlags);
}
//----------------------------------------------------------------------------
void FFiber::Shutdown() {
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());
    Assert(::GetCurrentFiber() == gCurrentThreadFiber);

    ::ConvertFiberToThread();
    gCurrentThreadFiber = nullptr;
}
//----------------------------------------------------------------------------
void* FFiber::ThreadFiber() {
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    return gCurrentThreadFiber;
}
//----------------------------------------------------------------------------
void* FFiber::RunningFiber() {
    Assert(gCurrentThreadFiber);
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
    const bool result = (nullptr != gCurrentThreadFiber);
    Assert(result == (TRUE == ::IsThreadAFiber()) );
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
