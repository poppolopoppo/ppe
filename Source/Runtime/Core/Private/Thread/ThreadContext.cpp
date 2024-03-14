// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Thread/ThreadContext.h"

#include "Thread/Fiber.h"

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

#include <atomic>

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Thread)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
THREAD_LOCAL size_t GCurrentThreadIndex = INDEX_NONE;
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
#if USE_PPE_ASSERT
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
    Unused(thread_id);
    Unused(name);
#endif
}
//----------------------------------------------------------------------------
static void UnregisterThreadName_(std::thread::id thread_id) {
#ifdef WITH_PPE_THREADCONTEXT_NAME
    FThreadNames_& thread_names = FThreadNames_::Get();
    WRITESCOPELOCK(thread_names.RWLock);
    thread_names.Names.Remove_AssertExists(thread_id);
    thread_names.Names.Vector().shrink_to_fit(); // release memory early-on
#else
    Unused(thread_id);
#endif
}
//----------------------------------------------------------------------------
static auto GetThreadHash_(std::thread::id id) {
    union UThreadIdIntegral_ {
        std::thread::id ThreadId;
        Meta::TIntegral<std::thread::id> Integral;
    };
    return UThreadIdIntegral_{ id }.Integral;
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& WriteThreadPriority_(TBasicTextWriter<_Char>& oss, EThreadPriority priority) {
    switch (priority)
    {
    case PPE::EThreadPriority::Realtime:
        oss << STRING_LITERAL(_Char, "Realtime");
        break;
    case PPE::EThreadPriority::Highest:
        oss << STRING_LITERAL(_Char, "Highest");
        break;
    case PPE::EThreadPriority::AboveNormal:
        oss << STRING_LITERAL(_Char, "AboveNormal");
        break;
    case PPE::EThreadPriority::Normal:
        oss << STRING_LITERAL(_Char, "Normal");
        break;
    case PPE::EThreadPriority::BelowNormal:
        oss << STRING_LITERAL(_Char, "BelowNormal");
        break;
    case PPE::EThreadPriority::Lowest:
        oss << STRING_LITERAL(_Char, "Lowest");
        break;
    case PPE::EThreadPriority::Idle:
        oss << STRING_LITERAL(_Char, "Idle");
        break;
    default:
        AssertNotImplemented();
    }
    return oss;
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

    PPE_LOG(Thread, Debug, "set thread {0} affinity mask to {1:#16b}", ThreadId(), mask);

    FPlatformThread::SetAffinityMask(mask);
}
//----------------------------------------------------------------------------
EThreadPriority FThreadContext::Priority() const {
    Assert(std::this_thread::get_id() == _threadId);

    return FPlatformThread::Priority();
}
//----------------------------------------------------------------------------
void FThreadContext::SetPriority(EThreadPriority priority) const {
    Assert(std::this_thread::get_id() == _threadId);

    PPE_LOG(Thread, Debug, "set thread {0} priority to {1}", ThreadId(), priority);

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
hash_t FThreadContext::ThreadOrFiberToken() NOEXCEPT {
    if (void* const fiber = FFiber::RunningFiberIFP())
        return hash_ptr(fiber);
    else
        return GetThreadHash_(std::this_thread::get_id());
}
//----------------------------------------------------------------------------
size_t FThreadContext::GetThreadHash(std::thread::id thread_id) NOEXCEPT {
    return checked_cast<size_t>(GetThreadHash_(thread_id));
}
//----------------------------------------------------------------------------
const char* FThreadContext::GetThreadName(std::thread::id thread_id) NOEXCEPT {
#ifdef WITH_PPE_THREADCONTEXT_NAME
    return GetThreadName_(thread_id).data();
#else
    Unused(thread_id);
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

#if USE_PPE_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    PPE_LOG(Thread, Debug, "start thread '{0}' with tag = {1} ({2})", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
#endif
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Start_MainThread() {
    FThreadLocalContext_::CreateMainThread();
    Meta::FThreadLocalAutoSingletonManager::Start();
    FAllocaStartup::Start(true);

#if USE_PPE_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    PPE_LOG(Thread, Debug, "start thread '{0}' with tag = {1} ({2}) <MainThread>", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
#endif

    FThreadContext& mainThread = FThreadLocalContext_::Get();
    mainThread.SetPriority(EThreadPriority::Realtime);
    mainThread.SetAffinityMask(FPlatformThread::MainThreadAffinity());
}
//----------------------------------------------------------------------------
void FThreadContextStartup::Shutdown() {
#if USE_PPE_LOGGER
    const FThreadContext& ctx = CurrentThreadContext();
    PPE_LOG(Thread, Debug, "stop thread '{0}' with tag = {1} ({2})", MakeCStringView(ctx.Name()), ctx.Tag(), ctx.ThreadId());
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
    return WriteThreadPriority_(oss, priority);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EThreadPriority priority) {
    return WriteThreadPriority_(oss, priority);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
