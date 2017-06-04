#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Heap.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHeap& GetThreadLocalHeap();
//----------------------------------------------------------------------------
template <typename T, typename _MemoryDomain>
struct TThreadLocalHeapFree {
    typedef void result_type;
    typedef T * argument_type;
    void operator ()(T * x) const {
        STATIC_ASSERT(Meta::TIsPod<T>::value); // ~T is never called !
        if (x)
            GetThreadLocalHeap().Free((void*)x, _MemoryDomain::TrackingData);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _MemoryDomain>
using TThreadLocalPtr = TUniquePtr<T, TThreadLocalHeapFree<T, _MemoryDomain> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FThreadLocalHeapStartup {
public:
    static void Start(bool mainThread);
    static void Shutdown();

    FThreadLocalHeapStartup(bool mainThread) { Start(mainThread); }
    ~FThreadLocalHeapStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
