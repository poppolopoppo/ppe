#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Heap.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Heap& GetThreadLocalHeap();
//----------------------------------------------------------------------------
template <typename T, typename _MemoryDomain>
struct ThreadLocalHeapFree {
    typedef void result_type;
    typedef T * argument_type;
    void operator ()(T * x) const {
        STATIC_ASSERT(std::is_pod<T>::value); // ~T is never called !
        if (x)
            GetThreadLocalHeap().Free(x, _MemoryDomain::TrackingData);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _MemoryDomain>
using ThreadLocalPtr = UniquePtr<T, ThreadLocalHeapFree<T, _MemoryDomain> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadLocalHeapStartup {
public:
    static void Start(bool mainThread);
    static void Shutdown();

    ThreadLocalHeapStartup(bool mainThread) { Start(mainThread); }
    ~ThreadLocalHeapStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
