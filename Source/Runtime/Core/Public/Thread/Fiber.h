#pragma once

#include "Core.h"

#if USE_PPE_ASSERT
#   include <atomic>
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFiber {
public:
    typedef void (STDCALL *callback_t)(void *arg);

    FFiber() NOEXCEPT : FFiber(nullptr) {}
    FFiber(void* pimpl) NOEXCEPT : _pimpl(pimpl) {}
    ~FFiber();

    FFiber(const FFiber& ) = delete;
    FFiber& operator =(const FFiber& ) = delete;

    FFiber(FFiber&& rvalue) NOEXCEPT;
    FFiber& operator =(FFiber&& rvalue) NOEXCEPT;

    void* Pimpl() const { return _pimpl; }

    void Create(callback_t entryPoint, void *arg, size_t stackSize = 0);
    void Resume() const;
    void Destroy(size_t stackSize);

    void Reset(void* pimpl = nullptr);

    void Swap(FFiber& other) { std::swap(_pimpl, other._pimpl); }

    void StackRegion(const void** pStackBottom, size_t* pStackSize) const NOEXCEPT;

    PPE_FAKEBOOL_OPERATOR_DECL() { return (!!_pimpl); }

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
} //!namespace PPE
