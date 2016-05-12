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
    typedef void (STDCALL *callback_t)(void *arg);

    Fiber() : Fiber(nullptr) {}
    Fiber(void* pimpl) : _pimpl(pimpl) {}
    ~Fiber();

    Fiber(const Fiber& ) = delete;
    Fiber& operator =(const Fiber& ) = delete;

    Fiber(Fiber&& rvalue);
    Fiber& operator =(Fiber&& rvalue);

    void* Pimpl() const { return _pimpl; }

    void Create(callback_t entryPoint, void *arg, size_t stackSize = 0);
    void Resume();
    void Destroy();

    void Reset(void* pimpl = nullptr);

    operator void *() const { return _pimpl; }

    bool operator ==(const Fiber& other) const { return _pimpl == other._pimpl; }
    bool operator !=(const Fiber& other) const { return !operator ==(other); }

    static void Start();
    static void Shutdown();

    static void* ThreadFiber();
    static void* RunningFiber();
    static void* RunningFiberIFP();

    static bool IsInFiber();

    struct ThreadScope {
        ThreadScope() { Start(); }
        ~ThreadScope() { Shutdown(); }
    };

private:
    void* _pimpl;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
