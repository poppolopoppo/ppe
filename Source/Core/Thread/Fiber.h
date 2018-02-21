#pragma once

#include "Core/Core.h"

#ifdef WITH_CORE_ASSERT
#   include <atomic>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFiber {
public:
    typedef void (STDCALL *callback_t)(void *arg);

    FFiber() : FFiber(nullptr) {}
    FFiber(void* pimpl) : _pimpl(pimpl) {}
    ~FFiber();

    FFiber(const FFiber& ) = delete;
    FFiber& operator =(const FFiber& ) = delete;

    FFiber(FFiber&& rvalue);
    FFiber& operator =(FFiber&& rvalue);

    void* Pimpl() const { return _pimpl; }

    void Create(callback_t entryPoint, void *arg, size_t stackSize = 0);
    void Resume();
    void Destroy(size_t stackSize);

    void Reset(void* pimpl = nullptr);

    void Swap(FFiber& other) { std::swap(_pimpl, other._pimpl); }

    CORE_FAKEBOOL_OPERATOR_DECL() { return _pimpl; }

    bool operator ==(const FFiber& other) const { return _pimpl == other._pimpl; }
    bool operator !=(const FFiber& other) const { return !operator ==(other); }

    static void Start();
    static void Shutdown();

    static void* ThreadFiber();
    static void* RunningFiber();
    static void* RunningFiberIFP();

    static bool IsInFiber();

    static void* CurrentFiberData();

    struct FThreadScope {
        FThreadScope() { Start(); }
        ~FThreadScope() { Shutdown(); }
    };

private:
    void* _pimpl;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
