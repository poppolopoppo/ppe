#include "stdafx.h"

#include "ThreadLocalHeap.h"

#include "Thread/ThreadLocalSingleton.h"

namespace Core
{
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class ThreadLocalHeap : Meta::ThreadLocalSingleton<Heap, ThreadLocalHeap> {
    typedef Meta::ThreadLocalSingleton<Heap, ThreadLocalHeap> parent_type;
public:
    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create();
    static void CreateMainThread();
};
//----------------------------------------------------------------------------
void ThreadLocalHeap::Create() {
    static const char* key = "ThreadLocalHeap";
    parent_type::Create(key, key, false /* not locked since accessed by only one thread */);
}
//----------------------------------------------------------------------------
void ThreadLocalHeap::CreateMainThread() {
    static const char* key = "ThreadLocalHeap";
    parent_type::Create(key, Heap::current_process_t() /* thread local in main thread <=> process heap */);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Heap& GetThreadLocalHeap() {
    return ThreadLocalHeap::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ThreadLocalHeapStartup::Start(bool mainThread) {
    if (mainThread)
        ThreadLocalHeap::CreateMainThread();
    else
        ThreadLocalHeap::Create();
}
//----------------------------------------------------------------------------
void ThreadLocalHeapStartup::Shutdown() {
    ThreadLocalHeap::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
