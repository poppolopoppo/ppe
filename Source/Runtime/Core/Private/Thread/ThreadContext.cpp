#include "stdafx.h"

#include "Thread/ThreadContext.h"

#include "Allocator/Alloca.h"
#include "Allocator/Malloc.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformProfiler.h"
#include "HAL/PlatformThread.h"
#include "Meta/AutoSingleton.h"
#include "Meta/NumericLimits.h"
#include "Meta/Singleton.h"

#if USE_PPE_PLATFORM_DEBUG
#   define WITH_PPE_THREADCONTEXT_NAME
#endif

#ifdef WITH_PPE_THREADCONTEXT_NAME
#   include "Container/AssociativeVector.h"
#   include "Thread/ReadWriteLock.h"
#endif

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Thread)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
thread_local size_t GCurrentThreadIndex = INDEX_NONE;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static std::atomic<size_t> GNumThreadContext_{ 0 };
//----------------------------------------------------------------------------
class FThreadLocalContext_ : Meta::TThreadLocalSingleton<FThreadContext, FThreadLocalContext_> {
    typedef Meta::TThreadLocalSingleton<FThreadContext, FThreadLocalContext_> parent_type;
public:
#ifdef WITH_PPE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Get;
    using parent_type::Destroy;

    static void Create(const char* name, size_t tag);
    static void CreateMainThread();
};
//----------------------------------------------------------------------------
void FThreadLocalContext_::Create(const char* name, size_t tag) {
    const size_t threadIndex = (GNumThreadContext_++);
    GCurrentThreadIndex = threadIndex;
    parent_type::Create(name, tag, threadIndex);
}
//----------------------------------------------------------------------------
void FThreadLocalContext_::CreateMainThread() {
    FThreadLocalContext_::Create("MainThread", PPE_THREADTAG_MAIN);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef WITH_PPE_THREADCONTEXT_NAME
struct FThreadNames_ {
    FReadWriteLock RWLock;
    ASSOCIATIVE_VECTORINSITU(Diagnostic, std::thread::id, FStringView, 32) Names;
    static FThreadNames_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FThreadNames_, GInstance);
        return GInstance;
    }
};
static FStringView GetThreadName_(std::thread::id thread_id) {
    FThreadNames_& thread_names = FThreadNames_::Get();
    READSCOPELOCK(thread_names.RWLock);
    const auto it = thread_names.Names.Find(thread_id);
    return (thread_names.Names.end() == it
        ? MakeStringView("UnkownThread")
        : it->second );
}
#endif //!WITH_PPE_THREADCONTEXT_NAME
//----------------------------------------------------------------------------
static void RegisterThreadName_(std::thread::id thread_id, const char* name) {
#ifdef WITH_PPE_THREADCONTEXT_NAME
#   if USE_PPE_PLATFORM_PROFILER
    FPlatformProfiler::Name(FPlatformProfiler::ThreadLevel, name);
#   endif
    FPlatformDebug::SetThreadDebugName(name);
    FThreadNames_& thread_names = FThreadNames_::Get();
    WRITESCOPELOCK(thread_names.RWLock);
    thread_names.Names.Insert_AssertUnique(thread_id, MakeCStringView(name));
#else
    UNUSED(thread_id);
    UNUSED(name);
#endif
}
//----------------------------------------------------------------------------
static void UnregisterThreadName_(std::thread::id thread_id) {
#ifdef WITH_PPE_THREADCONTEXT_NAME
    FThreadNames_& thread_names = FThreadNames_::Get();
    WRITESCOPELOCK(thread_names.RWLock);
    thread_names.Names.Remove_AssertExists(thread_id);
#else
    UNUSED(thread_id);
#endif
}
//----------------------------------------------------------------------------
static auto GetThreadHash_(std::thread::id id) {
    union UThreadIdIntegral_ {
        std::thread::id ThreadId;
        TIntegral<std::thread::id>::type Integral;
    };
    return UThreadIdIntegral_{ id }.Integral;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FThreadContext::FThreadContext(const char* name, size_t tag, size_t index)
:   _tag(tag)
,   _threadIndex(index)
,   _threadId(std::this_thread::get_id()) {
    Assert(name);

    const size_t n = Copy(MakeView(_name), MakeCStringView(name));
    Assert(n < lengthof(_name));
    _name[n] = '\0';

    RegisterThreadName_(_threadId, _name);

    FPlatformThread::OnThreadStart();
}
//----------------------------------------------------------------------------
FThreadContext::~FThreadContext() {
    FPlatformThread::OnThreadShutdown();

    UnregisterThreadName_(_threadId);
}
//----------------------------------------------------------------------------
u64 FThreadContext::AffinityMask() const {
    Assert(std::this_thread::get_id() == _threadId);

    return FPlatformThread::AffinityMask();
}
//----------------------------------------------------------------------------
void FThreadContext::SetAffinityMask(u64 mask) const {
    Assert(0 != mask);
    Assert(std::this_thread::get_id() == _threadId);

    LOG(Thread, Debug, L"set thread {0} affinity mask to {1:X}", ThreadId(), mask);

    FPlatformThread::SetAffinityMask(u64(mask));
}
//----------------------------------------------------------------------------
EThreadPriority FThreadContext::Priority() const {
    Assert(std::this_thread::get_id() == _threadId);

    return FPlatformThread::Priority();
}
//----------------------------------------------------------------------------
void FThreadContext::SetPriority(EThreadPriority priority) const {
    Assert(std::this_thread::get_id() == _threadId);

    LOG(Thread, Debug, L"set thread {0} priority to {1}", ThreadId(), priority);

    return FPlatformThread::SetPriority(priority);
}
//----------------------------------------------------------------------------
// you can put here everything you'll want to tick regularly
void FThreadContext::DutyCycle() const {
    Assert(std::this_thread::get_id() == _threadId);

    // chance to cleanup thread local allocator
    malloc_release_pending_blocks();
}
//----------------------------------------------------------------------------
size_t FThreadContext::NumThreads() {
    return GNumThreadContext_;
}
//----------------------------------------------------------------------------
size_t FThreadContext::GetThreadHash(std::thread::id thread_id) {
    return checked_cast<size_t>(GetThreadHash_(thread_id));
}
//----------------------------------------------------------------------------
const char* FThreadContext::GetThreadName(std::thread::id thread_id) {
#ifdef WITH_PPE_THREADCONTEXT_NAME
    return GetThreadName_(thread_id).data();
#else
    NOOP(thread_id);
    return nullptr;
#endif
}
//----------------------------------------------------------------------------
size_t FThreadContext::MaxThreadIndex() {
    return GNumThreadContext_;
}
//----------------------------------------------------------------------------
const FThreadContext& CurrentThreadContext() {
    return FThreadLocalContext_::Get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FThreadContextStartup::Start(const char* name, size_t tag) {
    FThreadLocalContext_::Create(name, tag);
    Meta::FThreadLocalAutoSingletonManager::Start();
    FAllocaStartup::Start(false);

    malloc_release_pending_blocks(); // force allocation of malloc TLS

#ifdef USE_DEBUG_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Thread, Debug, L"start thread '{0}' with tag = {1} ({2})", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
#endif
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Start_MainThread() {
    FThreadLocalContext_::CreateMainThread();
    Meta::FThreadLocalAutoSingletonManager::Start();
    FAllocaStartup::Start(true);

#ifdef USE_DEBUG_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Thread, Debug, L"start thread '{0}' with tag = {1} ({2}) <MainThread>", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
#endif

    FThreadContext& mainThread = FThreadLocalContext_::Get();
    mainThread.SetPriority(EThreadPriority::Realtime);
    mainThread.SetAffinityMask(FGenericPlatformThread::MainThreadAffinity);
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Shutdown() {
#ifdef USE_DEBUG_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    LOG(Thread, Debug, L"stop thread '{0}' with tag = {1} ({2})", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
#endif

    FAllocaStartup::Shutdown();
    FThreadLocalContext_::Destroy();
    Meta::FThreadLocalAutoSingletonManager::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, std::thread::id thread_id) {
#ifdef WITH_PPE_THREADCONTEXT_NAME
    return Format(oss, "thread_id:{0:#5} \"{1}\"", GetThreadHash_(thread_id), GetThreadName_(thread_id));
#else
    return Format(oss, "thread_id:{0:#5}", GetThreadHash_(thread_id));
#endif
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, std::thread::id thread_id) {
#ifdef WITH_PPE_THREADCONTEXT_NAME
    return Format(oss, L"thread_id:{0:#5} \"{1}\"", GetThreadHash_(thread_id), GetThreadName_(thread_id));
#else
    return Format(oss, L"thread_id:{0:#5}", GetThreadHash_(thread_id));
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EThreadPriority priority) {
    switch (priority)
    {
    case PPE::EThreadPriority::Realtime:
        oss << "Realtime";
        break;
    case PPE::EThreadPriority::Highest:
        oss << "Highest";
        break;
    case PPE::EThreadPriority::AboveNormal:
        oss << "AboveNormal";
        break;
    case PPE::EThreadPriority::Normal:
        oss << "Normal";
        break;
    case PPE::EThreadPriority::BelowNormal:
        oss << "BelowNormal";
        break;
    case PPE::EThreadPriority::Lowest:
        oss << "Lowest";
        break;
    case PPE::EThreadPriority::Idle:
        oss << "Idle";
        break;
    default:
        AssertNotImplemented();
    }
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EThreadPriority priority) {
    switch (priority)
    {
    case PPE::EThreadPriority::Realtime:
        oss << L"Realtime";
        break;
    case PPE::EThreadPriority::Highest:
        oss << L"Highest";
        break;
    case PPE::EThreadPriority::AboveNormal:
        oss << L"AboveNormal";
        break;
    case PPE::EThreadPriority::Normal:
        oss << L"Normal";
        break;
    case PPE::EThreadPriority::BelowNormal:
        oss << L"BelowNormal";
        break;
    case PPE::EThreadPriority::Lowest:
        oss << L"Lowest";
        break;
    case PPE::EThreadPriority::Idle:
        oss << L"Idle";
        break;
    default:
        AssertNotImplemented();
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
