#pragma once

#include "Core/Core.h"

#ifdef WITH_CORE_ASSERT
#   include <atomic>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Fiber {
public:
    typedef void (__stdcall *callback_t)(void *arg);

    Fiber(void *pimpl = nullptr) : _pimpl(pimpl) {}

    void Create(callback_t entryPoint, void *arg, size_t stackSize = 0);
    void Resume();
    void Destroy();

    operator void *() const { return _pimpl; }

    bool operator ==(const Fiber& other) const { return _pimpl == other._pimpl; }
    bool operator !=(const Fiber& other) const { return !operator ==(other); }

    static void Start();
    static void Shutdown();

    static Fiber ThreadFiber();
    static Fiber RunningFiber();
    static Fiber RunningFiberIFP();

    static bool IsInFiber();

    struct ThreadScope {
        ThreadScope() { Start(); }
        ~ThreadScope() { Shutdown(); }
    };

    void *_pimpl;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FiberFactory {
public:
    FiberFactory(Fiber::callback_t entryPoint, void *arg, size_t stackSize = 0);
    ~FiberFactory();

    FiberFactory(const FiberFactory& ) = delete;
    FiberFactory& operator =(const FiberFactory& ) = delete;

    Fiber::callback_t EntryPoint() const { return _entryPoint; }
    void *Arg() const { return _arg; }
    size_t StackSize() const { return _stackSize; }

    Fiber Create();
    void Release(Fiber& fiber);

private:
    const Fiber::callback_t _entryPoint;
    void *const _arg;
    const size_t _stackSize;

#ifdef WITH_CORE_ASSERT
    std::atomic<size_t> _count;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
