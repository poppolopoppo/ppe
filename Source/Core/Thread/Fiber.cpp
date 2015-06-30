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
void Fiber::Create(callback_t entryPoint, void *arg, size_t stackSize/* = 0 */) {
    Assert(!_pimpl);
    Assert(entryPoint);
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    if (0 == stackSize)
        stackSize = FiberStackReserveSize;

#if 0
    _pimpl = ::CreateFiber(stackSize, entryPoint, arg);
#else
    _pimpl = ::CreateFiberEx(   FiberStackCommitSize,
                                stackSize,
                                FiberFlags,
                                entryPoint, arg );
#endif

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
Fiber Fiber::ThreadFiber() {
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    return gCurrentThreadFiber;
}
//----------------------------------------------------------------------------
Fiber Fiber::RunningFiber() {
    Assert(gCurrentThreadFiber);
    Assert(::IsThreadAFiber());

    return ::GetCurrentFiber();
}
//----------------------------------------------------------------------------
Fiber Fiber::RunningFiberIFP() {
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
FiberFactory::FiberFactory(Fiber::callback_t entryPoint, void *arg, size_t stackSize/* = 0 */) 
:   _entryPoint(entryPoint)
,   _arg(arg)
,   _stackSize(stackSize)
#ifdef WITH_CORE_ASSERT
,   _count(0)
#endif
{
    Assert(_entryPoint);
}
//----------------------------------------------------------------------------
FiberFactory::~FiberFactory() {
#ifdef WITH_CORE_ASSERT
    Assert(0 == _count);
#endif
}
//----------------------------------------------------------------------------
Fiber FiberFactory::Create() {
    Fiber result;
    result.Create(_entryPoint, _arg, _stackSize);

#ifdef WITH_CORE_ASSERT
    ++_count;
#endif
    
    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
void FiberFactory::Release(Fiber& fiber) {
    Assert(fiber);
    Assert(Fiber::RunningFiber() != fiber);

    fiber.Destroy();

#ifdef WITH_CORE_ASSERT
    Assert(_count);
    --_count;
    fiber = Fiber();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core