#include "stdafx.h"

#include "Diagnostic/Logger.h"

#if USE_PPE_LOGGER

#   include "Allocator/LinearHeap.h"
#   include "Allocator/LinearAllocator.h"
#   include "Allocator/TrackingMalloc.h"
#   include "Container/Vector.h"
#   include "Diagnostic/CurrentProcess.h"
#   include "HAL/PlatformConsole.h"
#   include "HAL/PlatformDebug.h"
#   include "HAL/PlatformFile.h"
#   include "HAL/PlatformMemory.h"
#   include "IO/BufferedStream.h"
#   include "IO/FileSystem.h"
#   include "IO/FileStream.h"
#   include "IO/FormatHelpers.h"
#   include "IO/StreamProvider.h"
#   include "IO/String.h"
#   include "IO/StringBuilder.h"
#   include "IO/StringView.h"
#   include "IO/TextWriter.h"
#   include "Memory/InSituPtr.h"
#   include "Memory/MemoryView.h"
#   include "Memory/UniquePtr.h"
#   include "Meta/Optional.h"
#   include "Meta/Singleton.h"
#   include "Meta/ThreadResource.h"
#   include "Thread/AtomicSpinLock.h"
#   include "Thread/Task/TaskManager.h"
#   include "Thread/Task/TaskHelpers.h"
#   include "Thread/Fiber.h"
#   include "Thread/ThreadContext.h"
#   include "Thread/ThreadPool.h"
#   include "Time/DateTime.h"

#   include <atomic>
#   include <mutex>
#   include <iostream>

#   define PPE_DUMP_THREAD_ID              1
#   define PPE_DUMP_THREAD_NAME            0
#   define PPE_DUMP_SITE_ON_LOG            0
#   define PPE_DUMP_CALLSTACK_ON_WARNING   0
#   define PPE_DUMP_CALLSTACK_ON_ERROR     0

#   if (PPE_DUMP_CALLSTACK_ON_ERROR || PPE_DUMP_CALLSTACK_ON_WARNING)
#       include "Diagnostic/Callstack.h"
#       include "Diagnostic/DecodedCallstack.h"
#   endif

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, LogDefault)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static THREAD_LOCAL bool GIsInLogger_ = false;
struct FIsInLoggerScope {
    const bool WasInLogger;
    FIsInLoggerScope() : WasInLogger(GIsInLogger_) { GIsInLogger_ = true; }
    ~FIsInLoggerScope() { GIsInLogger_ = WasInLogger; }
};
//----------------------------------------------------------------------------
struct FLoggerTypes {
    using EVerbosity = ILogger::EVerbosity;
    using FCategory = ILogger::FCategory;
    using FSiteInfo = ILogger::FSiteInfo;
};
//----------------------------------------------------------------------------
class ILowLevelLogger : public FLoggerTypes {
public:
    virtual ~ILowLevelLogger() = default;

    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) = 0;
    virtual void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args) = 0;
    virtual void Flush(bool synchronous) = 0;
};
//----------------------------------------------------------------------------
// Use a custom allocator for logger to diminish content, fragmentation and get re-entrancy
class FLogAllocator : public Meta::TSingleton<FLogAllocator> {
    friend class Meta::TSingleton<FLogAllocator>;
    using singleton_type = Meta::TSingleton<FLogAllocator>;
public:
    using singleton_type::Create;
    using singleton_type::Destroy;
    using singleton_type::Get;

    struct FBucket {
        std::recursive_mutex Barrier;
        LINEARHEAP_POOLED(Logger) Heap;
        FBucket() = default;
        ~FBucket() {
            const Meta::FRecursiveLockGuard scopeLock(Barrier);
            Heap.ReleaseAll();
        }
    };

    struct FScope {
        size_t Index;
        FBucket& Bucket;
        Meta::FRecursiveLockGuard Lock;

        FPooledLinearHeap& Heap() const { return Bucket.Heap; }
        operator FLinearAllocator () const NOEXCEPT { return { Bucket.Heap }; }

        FScope() : FScope(Get()) {}

        explicit FScope(size_t index)
            : Index(index)
            , Bucket(Get().OpenBucket_(Index))
            , Lock(Bucket.Barrier)
        {}

        explicit FScope(FLogAllocator& alloc)
            : Index(alloc.NextBucketIndex_())
            , Bucket(alloc.OpenBucket_(Index))
            , Lock(Bucket.Barrier)
        {}
    };

    void TrimCache() {
        forrange(i, 0, NumAllocationBuckets) {
            const Meta::FRecursiveLockGuard scopeLock(_buckets[i].Barrier);
            _buckets[i].Heap.TrimPools();
        }
    }

private:
    STATIC_CONST_INTEGRAL(size_t, NumAllocationBuckets, 8);
    STATIC_CONST_INTEGRAL(size_t, MaskAllocationBuckets, NumAllocationBuckets - 1);
    STATIC_ASSERT(Meta::IsPow2(NumAllocationBuckets));

    std::atomic<size_t> _revision;
    FBucket _buckets[NumAllocationBuckets];

    FLogAllocator()
        : _revision(0)
    {}

    FBucket& OpenBucket_(size_t index) {
        Assert(index < NumAllocationBuckets);
        return _buckets[index];
    }

    size_t NextBucketIndex_() {
        return (_revision++ & MaskAllocationBuckets);
    }
};
//----------------------------------------------------------------------------
// Composite for all loggers supplied through public API
class FUserLogger final : Meta::TSingleton<FUserLogger>, public ILowLevelLogger {
    friend class Meta::TSingleton<FUserLogger>;
    using singleton_type = Meta::TSingleton<FUserLogger>;
public:
    using singleton_type::Create;
    using singleton_type::Destroy;
    using singleton_type::Get;

    void Add(const PLogger& logger) {
        Assert(logger);

        const Meta::FLockGuard scopeLock(_barrier);
        Assert(not Contains(_loggers, logger));
        _loggers.emplace_back(logger);
    }

    void Remove(const PLogger& logger) {
        Assert(logger);

        const Meta::FLockGuard scopeLock(_barrier);
        const auto it = std::find(_loggers.begin(), _loggers.end(), logger);
        Assert(_loggers.end() != it);
        _loggers.erase(it);
    }

public: // ILowLevelLogger
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        const FReentrancyProtection_ scopeReentrant;
        if (scopeReentrant.WasLocked)
            return;

        const Meta::FLockGuard scopeLock(_barrier);
        for (const PLogger& logger : _loggers)
            logger->Log(category, level, site, text);
    }

    virtual void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args) override final {
        if (FReentrancyProtection_::GLockTLS) // skip Format() if already locked, but don't acquire the lock here
            return;

        const FLogAllocator::FScope scopeAlloc; // #TODO : could be a dead lock issue to keep the lock open while dispatching

        MEMORYSTREAM_LINEARHEAP() buf(scopeAlloc);
        FWTextWriter oss(&buf);

        FormatArgs(oss, format, args);

        Log(category, level, site, buf.MakeView().Cast<const wchar_t>());
    }

    virtual void Flush(bool synchronous) override final {
        const FReentrancyProtection_ scopeReentrant;
        if (scopeReentrant.WasLocked)
            return;

        const Meta::FLockGuard scopeLock(_barrier);
        for (const PLogger& logger : _loggers)
            logger->Flush(synchronous);
    }

private:
    // Need to avoid re-entrance when we're not using FBackgroundLogger as the front-end
    struct FReentrancyProtection_ {
        static THREAD_LOCAL bool GLockTLS;
        const bool WasLocked;
        FReentrancyProtection_()
        :   WasLocked(GLockTLS) {
            GLockTLS = true;
        }
        ~FReentrancyProtection_() {
            GLockTLS = WasLocked;
        }
    };

    FUserLogger() {} // private ctor

    std::mutex _barrier;
    VECTORINSITU(Logger, PLogger, 8) _loggers;
};
THREAD_LOCAL bool FUserLogger::FReentrancyProtection_::GLockTLS{ false };
//----------------------------------------------------------------------------
// Asynchronous logger used during the game
class FBackgroundLogger final : Meta::TSingleton<FBackgroundLogger>, public ILowLevelLogger {
    friend class Meta::TSingleton<FBackgroundLogger>;
    using singleton_type = Meta::TSingleton<FBackgroundLogger>;
public:
    using singleton_type::Create;
    using singleton_type::Destroy;
    using singleton_type::Get;

public: // ILowLevelLogger
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        FDeferredLog* log;
        { // don't lock both allocator & task manager to avoid dead locking
            const FLogAllocator::FScope scopeAlloc;

            const size_t sizeInBytes = (sizeof(FDeferredLog) + text.SizeInBytes());
            log = INPLACE_NEW(scopeAlloc.Heap().Allocate(sizeInBytes), FDeferredLog)(scopeAlloc.Heap());

            log->LowLevelLogger = _userLogger;
            log->Category = &category;
            log->Level = level;
            log->Site = site;
            log->TextLength = checked_cast<u32>(text.size());
            log->Bucket = checked_cast<u32>(scopeAlloc.Index);
            log->AllocSizeInBytes = checked_cast<u32>(sizeInBytes);
        }
        FPlatformMemory::Memcpy(log + 1, text.data(), text.SizeInBytes());

        TaskManager_().Run(MakeFunction<&FDeferredLog::Log>(log));
    }

    virtual void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args) override final {
        FDeferredLog* log;
        { // don't lock both allocator & task manager to avoid dead locking
            const FLogAllocator::FScope scopeAlloc;

            MEMORYSTREAM_LINEARHEAP() buf(scopeAlloc);
            buf.reserve(sizeof(FDeferredLog) + format.SizeInBytes());
            buf.resize(sizeof(FDeferredLog)); // reserve space for FDeferredLog entry
            buf.SeekO(0, ESeekOrigin::End); // seek at the end of the stream
            {
                FWTextWriter oss(&buf);
                FormatArgs(oss, format, args);
            }

            log = INPLACE_NEW(buf.Pointer(), FDeferredLog)(scopeAlloc.Heap());

            size_t sizeInBytes = 0;
            const FAllocatorBlock stolen = buf.StealDataUnsafe(&sizeInBytes);
            Assert_NoAssume(stolen.Data == (void*)log);
            Verify(log->Acquire(stolen));

            log->LowLevelLogger = _userLogger;
            log->Category = &category;
            log->Level = level;
            log->Site = site;
            log->TextLength = checked_cast<u32>((sizeInBytes - sizeof(FDeferredLog)) / sizeof(wchar_t));
            log->Bucket = checked_cast<u32>(scopeAlloc.Index);
            log->AllocSizeInBytes = checked_cast<u32>(stolen.SizeInBytes);
        }

        TaskManager_().Run(MakeFunction<&FDeferredLog::Log>(log));
    }

    virtual void Flush(bool synchronous) override final {
        if (synchronous && not GIsInLogger_) {
            TaskManager_().WaitForAll(1000/* 1 second */); // wait for all potential logs before flushing
            _userLogger->Flush(true);
        }
        else {
            TaskManager_().Run([logger{ _userLogger }](ITaskContext&) {
                logger->Flush(true); // flush synchronously in asynchronous task
            },  ETaskPriority::Low );
        }

        // good idea to trim the linear heaps when flushing
        FLogAllocator::Get().TrimCache();
    }

private:
    FUserLogger* const _userLogger;

    FBackgroundLogger()
        : _userLogger(&FUserLogger::Get())
    {}

    ~FBackgroundLogger() {
        TaskManager_().WaitForAll(); // blocking wait before destroying, avoid necrophilia
    }

    static FTaskManager& TaskManager_() {
        return FBackgroundThreadPool::Get();
    }

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
    class ALIGN(ALLOCATION_BOUNDARY) FDeferredLog : public FLinearAllocator, Meta::FNonCopyableNorMovable {
    public:
        ILowLevelLogger* LowLevelLogger;
        const FCategory* Category;
        FSiteInfo Site;
        EVerbosity Level;

        u32 TextLength;

        u32 Bucket;
        u32 AllocSizeInBytes;

#   if USE_PPE_ASSERT
        uintptr_t Canary = PPE_HASH_VALUE_SEED;
#   endif

        explicit FDeferredLog(FPooledLinearHeap& heap) NOEXCEPT
            : FLinearAllocator(heap)
        {}

        FWStringView Text() const { return { (const wchar_t *)(this + 1), TextLength }; }

        void Log(ITaskContext&) {
            const FSuicideScope_ logScope(this);
            LowLevelLogger->Log(*Category, Level, Site, Text());
        }
    };
    STATIC_ASSERT(std::is_trivially_destructible_v<FDeferredLog>);
PRAGMA_MSVC_WARNING_POP()

    // used for exception safety :
    struct FSuicideScope_ : FLogAllocator::FScope {
        FDeferredLog* Log;
        FSuicideScope_(FDeferredLog* log)
            : FLogAllocator::FScope(log->Bucket)
            , Log(log) {
            Assert(log);
            Assert_NoAssume(PPE_HASH_VALUE_SEED == Log->Canary);
            ONLY_IF_ASSERT(Log->Canary = uintptr_t(this));
        }
        ~FSuicideScope_() {
            Assert_NoAssume(uintptr_t(this) == Log->Canary);
            ONLY_IF_ASSERT(Log->Canary = 0);
            Heap().Deallocate(Log, Log->AllocSizeInBytes);
        }
    };
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static ILowLevelLogger* GLoggerImpl_ = nullptr;
//----------------------------------------------------------------------------
static void SetupLoggerImpl_(ILowLevelLogger* pimpl) {
    ONE_TIME_DEFAULT_INITIALIZE(FAtomicSpinLock, GBarrier);

    const FAtomicSpinLock::FScope scopeLock(GBarrier);

    std::swap(pimpl, GLoggerImpl_);

    if (pimpl)
        pimpl->Flush(true);
}
//----------------------------------------------------------------------------
class FLogFormat {
public:
    static void Header(FWTextWriter& oss, const ILogger::FCategory& category, ILogger::EVerbosity level, const ILogger::FSiteInfo& site) {
#if PPE_DUMP_THREAD_ID
#   if PPE_DUMP_THREAD_NAME
        Format(oss, L"[{0}][{1:20}][{3:-8}][{2:-15}] ", site.Timestamp.ToDateTimeUTC(), FThreadContext::GetThreadName(site.ThreadId), MakeCStringView(category.Name), level);
#   else // only thread hash :
        Format(oss, L"[{0}][{1:#5}][{3:-8}][{2:-15}] ", site.Timestamp.ToDateTimeUTC(), FThreadContext::GetThreadHash(site.ThreadId), MakeCStringView(category.Name), level);
#   endif
#else
        Format(oss, L"[{0}][{2:-8}][{1:-15}] ", site.Timestamp.ToDateTimeUTC(), MakeCStringView(category.Name), level);
#endif
    }

    static void Footer(FWTextWriter& oss, const ILogger::FCategory&, ILogger::EVerbosity level, const ILogger::FSiteInfo& site) {
#if PPE_DUMP_SITE_ON_LOG
        Format(oss, L"\n\tat {0}:{1}\n", site.Filename, site.Line);
#else
        UNUSED(level);
        UNUSED(site);
        oss << Eol;
#endif
    }

    static void Print(FWTextWriter& oss, const ILogger::FCategory& category, ILogger::EVerbosity level, const ILogger::FSiteInfo& site, const FWStringView& text) {
        Header(oss, category, level, site);
        oss.Write(text);
        Footer(oss, category, level, site);
    }
};
//----------------------------------------------------------------------------
// Used before & after main when no debugger is attached, ignores everything
class FDevNullLogger : public ILowLevelLogger {
public:
    static ILowLevelLogger* Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FDevNullLogger, GInstance);
        return (&GInstance);
    }

public: // ILowLevelLogger
    virtual void Log(const FCategory&, EVerbosity, const FSiteInfo&, const FWStringView&) override final {}
    virtual void LogArgs(const FCategory&, EVerbosity, const FSiteInfo&, const FWStringView&, const FWFormatArgList&) override final {}
    virtual void Flush(bool) override final {}
};
//----------------------------------------------------------------------------
// Used before & after main when debugger attached, no dependencies on allocators, always immediate
#if USE_PPE_PLATFORM_DEBUG
class FDebuggingLogger final : public ILowLevelLogger {
public:
    static ILowLevelLogger* Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FDebuggingLogger, GInstance);
        return (&GInstance);
    }

public: // ILowLevelLogger
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        Assert(category.Verbosity ^ level);

        wchar_t tmp[8192];
        FWFixedSizeTextWriter oss(tmp);

        FLogFormat::Header(oss, category, level, site);
        oss.Write(text);
        FLogFormat::Footer(oss, category, level, site);

        oss << Eos;

        FPlatformDebug::OutputDebug(tmp);
    }

    virtual void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args) override final {
        Assert(category.Verbosity ^ level);

        wchar_t tmp[8192];
        FWFixedSizeTextWriter oss(tmp);

        FLogFormat::Header(oss, category, level, site);
        FormatArgs(oss, format, args);
        FLogFormat::Footer(oss, category, level, site);

        oss << Eos;

        FPlatformDebug::OutputDebug(tmp);
    }

    virtual void Flush(bool) override final {} // always synchronous => no need to flush
};
#endif //!USE_PPE_PLATFORM_DEBUG
//----------------------------------------------------------------------------
static ILowLevelLogger* LowLevelLogger_() {
#if USE_PPE_PLATFORM_DEBUG
    return (FPlatformDebug::IsDebuggerPresent()
        ? FDebuggingLogger::Get()
        : FDevNullLogger::Get() );
#else
    return FDevNullLogger::Get();
#endif
}
//----------------------------------------------------------------------------
static NO_INLINE void SetupLowLevelLoggerImpl_() {
    SetupLoggerImpl_(LowLevelLogger_());
}
//----------------------------------------------------------------------------
static ILowLevelLogger& CurrentLogger_() {
    // fall back for pre-logger start
    if (Unlikely(nullptr == GLoggerImpl_))
        SetupLowLevelLoggerImpl_();

    return (*GLoggerImpl_);
}
//----------------------------------------------------------------------------
static void HandleFatalLogIFN_(FLogger::EVerbosity level) {
    if (FLogger::EVerbosity::Fatal == level) {
        CurrentLogger_().Flush(true);
        AssertNotReached();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLogger::Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) {
    const FIsInLoggerScope loggerScope;

    if (category.Verbosity ^ level)
        CurrentLogger_().Log(category, level, site, text);

    HandleFatalLogIFN_(level);
}
//----------------------------------------------------------------------------
void FLogger::LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args) {
    const FIsInLoggerScope loggerScope;

    if (category.Verbosity ^ level)
        CurrentLogger_().LogArgs(category, level, site, format, args);

    HandleFatalLogIFN_(level);
}
//----------------------------------------------------------------------------
void FLogger::Start() {
    FLogAllocator::Create();

    FUserLogger::Create();

    const auto& proc = FCurrentProcess::Get();

    // don't create a log file when running with an attached debugger
    if (proc.StartedWithDebugger()) {
        RegisterLogger(MakeOutputDebug());
    }
    else {
        RegisterLogger(MakeStdout());

        const FWString logPath = FPlatformFile::JoinPath({
        proc.SavedPath(), L"Log", MakeStringView(WSTRINGIZE(BUILDCONFIG)) });

        VerifyRelease(FPlatformFile::CreateDirectoryRecursively(*logPath, nullptr));

        const FWString logFile = FPlatformFile::JoinPath({
            logPath, proc.ExecutableName() + L".log" });

        RegisterLogger(FLogger::MakeRollFile(*logFile));

    }

    IF_CONSTEXPR(USE_PPE_MEMORY_DEBUGGING) {
        SetupLoggerImpl_(&FUserLogger::Get());
    }
    else {
        FBackgroundLogger::Create();
        SetupLoggerImpl_(&FBackgroundLogger::Get());
    }
}
//----------------------------------------------------------------------------
void FLogger::Shutdown() {
    SetupLowLevelLoggerImpl_();

    IF_CONSTEXPR(not USE_PPE_MEMORY_DEBUGGING) {
        FBackgroundLogger::Destroy();
    }

    FUserLogger::Destroy();

    FLogAllocator::Destroy();
}
//----------------------------------------------------------------------------
void FLogger::Flush(bool synchronous/* = true */) {
    CurrentLogger_().Flush(synchronous);
}
//----------------------------------------------------------------------------
void FLogger::RegisterLogger(const PLogger& logger) {
    if (logger)
        FUserLogger::Get().Add(logger);
}
//----------------------------------------------------------------------------
void FLogger::UnregisterLogger(const PLogger& logger) {
    Assert(logger);

    FUserLogger::Get().Remove(logger);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_PLATFORM_DEBUG
class FOutputDebugLogger_ final : public ILogger {
public:
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        FWStringBuilder oss(text.size());
        FLogFormat::Print(oss, category, level, site, text);
        oss << Eos;
        FPlatformDebug::OutputDebug(oss.Written().data());
    }

    virtual void Flush(bool) override final {} // always synched
};
#endif //!USE_PPE_PLATFORM_DEBUG
//----------------------------------------------------------------------------
class FStdoutLogger_ final : public ILogger {
public:
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        FWStringBuilder oss(text.size());
        FLogFormat::Print(oss, category, level, site, text);
        oss << Eol << Eos;
        ::fputws(oss.Written().data(), stdout);
    }

    virtual void Flush(bool) override final {
        fflush(stdout);
    }
};
//----------------------------------------------------------------------------
class FFunctorLogger_ final : public ILogger {
public:
    typedef TFunction<void(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&)> functor_type;

    explicit FFunctorLogger_(functor_type&& func)
        : _func(std::move(func))
    {}

public: // ILogger
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        _func(category, level, site, text);
    }

    virtual void Flush(bool) override final {} // always synced

private:
    functor_type _func;
};
//----------------------------------------------------------------------------
class FConsoleWriterLogger_ final : public ILogger {
public:
    FConsoleWriterLogger_() {
        FPlatformConsole::Open();
    }

    ~FConsoleWriterLogger_() {
        FPlatformConsole::Close();
    }

public: // ILogger
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        FWStringBuilder oss(text.size());
        FLogFormat::Print(oss, category, level, site, text);

        FPlatformConsole::EAttribute attrs;

        switch (level) {
        case ELoggerVerbosity::Info:
            attrs = FPlatformConsole::WHITE_ON_BLACK;
            break;
        case ELoggerVerbosity::Emphasis:
            attrs = (FPlatformConsole::FG_GREEN | FPlatformConsole::BG_BLUE | FPlatformConsole::FG_INTENSITY);
            break;
        case ELoggerVerbosity::Warning:
            attrs = (FPlatformConsole::FG_YELLOW | FPlatformConsole::BG_BLACK | FPlatformConsole::FG_INTENSITY);
            break;
        case ELoggerVerbosity::Error:
            attrs = (FPlatformConsole::FG_RED | FPlatformConsole::BG_BLACK);
            break;
        case ELoggerVerbosity::Debug:
            attrs = (FPlatformConsole::FG_CYAN | FPlatformConsole::BG_BLACK);
            break;
        case ELoggerVerbosity::Fatal:
            attrs = (FPlatformConsole::FG_WHITE | FPlatformConsole::BG_RED | FPlatformConsole::BG_INTENSITY);
            break;
        default:
            attrs = (FPlatformConsole::FG_WHITE | FPlatformConsole::BG_BLACK);
            break;
        }

        FPlatformConsole::Write(oss.Written(), attrs);
    }

    virtual void Flush(bool) override final {}
};
//----------------------------------------------------------------------------
class FFileHandleLogger_ final : public ILogger, IStreamWriter {
public:
    explicit FFileHandleLogger_(FPlatformLowLevelIO::FHandle hFile)
    :   _hFile(hFile)
    ,   _buffered(this) {
        Assert(FPlatformLowLevelIO::InvalidHandle != _hFile);
    }

    virtual ~FFileHandleLogger_() {
        _buffered.ResetStream();
        FPlatformLowLevelIO::Close(_hFile);
    }

public: // ILogger
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        FWTextWriter oss(&_buffered);
        FLogFormat::Print(oss, category, level, site, text);
    }

    virtual void Flush(bool) override final {
        _buffered.Flush();
    }

public: // IStreamWriter, implemented to be low level and without calls to LOG()
    virtual bool IsSeekableO(ESeekOrigin) const override final { return true; }

    virtual std::streamoff TellO() const { return FPlatformLowLevelIO::Tell(_hFile); }
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final {
        return FPlatformLowLevelIO::Seek(_hFile, offset, origin);
    }

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final {
        return (sizeInBytes == FPlatformLowLevelIO::Write(_hFile, storage, sizeInBytes));
    }
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final {
        std::streamsize written = checked_cast<std::streamsize>(eltsize * count);
        written = FPlatformLowLevelIO::Write(_hFile, storage, written);
        return checked_cast<size_t>(written / eltsize);
    }

    virtual class IBufferedStreamWriter* ToBufferedO() override final { return nullptr; }

private:
    FPlatformLowLevelIO::FHandle _hFile;
    FBufferedStreamWriter _buffered;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PLogger FLogger::MakeStdout() {
    return (FPlatformConsole::HasConsole
        ? PLogger(NEW_REF(Logger, FConsoleWriterLogger_))
        : PLogger(NEW_REF(Logger, FStdoutLogger_)) );
}
//----------------------------------------------------------------------------
PLogger FLogger::MakeOutputDebug() {
#if USE_PPE_PLATFORM_DEBUG
    return NEW_REF(Logger, FOutputDebugLogger_);
#else
    return PLogger();
#endif
}
//----------------------------------------------------------------------------
PLogger FLogger::MakeAppendFile(const wchar_t* filename) {
    FPlatformLowLevelIO::FHandle hFile = FPlatformLowLevelIO::Open(filename,
            EOpenPolicy::Writable,
            EAccessPolicy::Create|EAccessPolicy::Append|EAccessPolicy::Binary|EAccessPolicy::ShareRead);
    AssertRelease(FPlatformLowLevelIO::InvalidHandle != hFile);
    return NEW_REF(Logger, FFileHandleLogger_)(hFile);
}
//----------------------------------------------------------------------------
PLogger FLogger::MakeRollFile(const wchar_t* filename) {
    Verify(FPlatformFile::RollFile(filename));
    return MakeAppendFile(filename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, FLogger::EVerbosity level) {
    auto sep = Fmt::NotFirstTime('|');
    if (level & FLogger::EVerbosity::Debug)     oss << sep << "Debug";
    if (level & FLogger::EVerbosity::Verbose)   oss << sep << "Verbose";
    if (level & FLogger::EVerbosity::Info)      oss << sep << "Info";
    if (level & FLogger::EVerbosity::Emphasis)  oss << sep << "Emphasis";
    if (level & FLogger::EVerbosity::Warning)   oss << sep << "Warning";
    if (level & FLogger::EVerbosity::Error)     oss << sep << "Error";
    if (level & FLogger::EVerbosity::Fatal)     oss << sep << "Fatal";
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, FLogger::EVerbosity level) {
    auto sep = Fmt::NotFirstTime(L'|');
    if (level & FLogger::EVerbosity::Debug)     oss << sep << L"Debug";
    if (level & FLogger::EVerbosity::Verbose)   oss << sep << L"Verbose";
    if (level & FLogger::EVerbosity::Info)      oss << sep << L"Info";
    if (level & FLogger::EVerbosity::Emphasis)  oss << sep << L"Emphasis";
    if (level & FLogger::EVerbosity::Warning)   oss << sep << L"Warning";
    if (level & FLogger::EVerbosity::Error)     oss << sep << L"Error";
    if (level & FLogger::EVerbosity::Fatal)     oss << sep << L"Fatal";
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_LOGGER
