#include "stdafx.h"

#include "Fiber.h"

#ifdef OS_WINDOWS
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
Fiber::~Fiber() {
    if (_pimpl)
        Destroy();
}
//----------------------------------------------------------------------------
Fiber::Fiber(Fiber&& rvalue)
    : _pimpl(rvalue._pimpl) {
    rvalue._pimpl = nullptr;
}
//----------------------------------------------------------------------------
Fiber& Fiber::operator =(Fiber&& rvalue) {
    if (_pimpl)
        Destroy();

    Assert(nullptr == _pimpl);
    std::swap(_pimpl, rvalue._pimpl);

    return *this;
}
//----------------------------------------------------------------------------
void Fiber::Create(callback_t entryPoint, void *arg, size_t stackSize/* = 0 */) {
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
void Fiber::Resume() {
    Assert(_pimpl);
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());
    Assert(::GetCurrentFiber() != _pimpl);

    ::SwitchToFiber(_pimpl);
}
//----------------------------------------------------------------------------
void Fiber::Destroy() {
    Assert(_pimpl);
    Assert(::GetCurrentFiber() != _pimpl);

    ::DeleteFiber(_pimpl);
    _pimpl = nullptr;
}
//----------------------------------------------------------------------------
void Fiber::Reset(void* pimpl /* = nullptr */) {
    _pimpl = pimpl;
}
//----------------------------------------------------------------------------
void Fiber::Start() {
    Assert(!gCurrentThreadFiber);
    Assert(!::IsThreadAFiber());

    gCurrentThreadFiber = ::ConvertThreadToFiberEx(nullptr, FiberFlags);
}
//----------------------------------------------------------------------------
void Fiber::Shutdown() {
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());
    Assert(::GetCurrentFiber() == gCurrentThreadFiber);

    ::ConvertFiberToThread();
    gCurrentThreadFiber = nullptr;
}
//----------------------------------------------------------------------------
void* Fiber::ThreadFiber() {
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    return gCurrentThreadFiber;
}
//----------------------------------------------------------------------------
void* Fiber::RunningFiber() {
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    return ::GetCurrentFiber();
}
//----------------------------------------------------------------------------
void* Fiber::RunningFiberIFP() {
    return ::IsThreadAFiber()
        ? ::GetCurrentFiber()
        : nullptr;
}
//----------------------------------------------------------------------------
bool Fiber::IsInFiber() {
    const bool result = (nullptr != gCurrentThreadFiber);
    Assert(result == (TRUE == ::IsThreadAFiber()) );
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core