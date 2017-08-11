#include "stdafx.h"

#include "ThreadLocalHeap.h"

#include "Meta/Singleton.h"

namespace Core
{
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FThreadLocalHeap : Meta::TThreadLocalSingleton<FHeap, FThreadLocalHeap> {
    typedef Meta::TThreadLocalSingleton<FHeap, FThreadLocalHeap> parent_type;
public:
    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create();
    static void CreateMainThread();
};
//----------------------------------------------------------------------------
void FThreadLocalHeap::Create() {
    parent_type::Create();
}
//----------------------------------------------------------------------------
void FThreadLocalHeap::CreateMainThread() {
    parent_type::Create(FHeap::current_process_t{});
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHeap& GetThreadLocalHeap() {
    return FThreadLocalHeap::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FThreadLocalHeapStartup::Start(bool mainThread) {
    if (mainThread)
        FThreadLocalHeap::CreateMainThread();
    else
        FThreadLocalHeap::Create();
}
//----------------------------------------------------------------------------
void FThreadLocalHeapStartup::Shutdown() {
    FThreadLocalHeap::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
