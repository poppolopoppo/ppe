#include "stdafx.h"

#include "Diagnostic/Logger.h"

#ifdef USE_DEBUG_LOGGER

#   include "Allocator/LinearHeap.h"
#   include "Allocator/LinearHeapAllocator.h"
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
    virtual ~ILowLevelLogger() {}

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

    struct FBucket : TLinearHeapAllocator<u8> {
        std::recursive_mutex Barrier;
        LINEARHEAP(Logger) Heap;
        FBucket() : TLinearHeapAllocator<u8>(Heap) {}
        ~FBucket() {
            const Meta::FRecursiveLockGuard scopeLock(Barrier);
            Heap.ReleaseAll();
        }
    };

    struct FScope {
        size_t Index;
        FBucket& Bucket;
        Meta::FRecursiveLockGuard Lock;

        FLinearHeap& Heap() const { return Bucket.Heap; }

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

        const FLogAllocator::FScope scopeAlloc;

        MEMORYSTREAM_LINEARHEAP() buf(scopeAlloc.Heap());
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
    // Need to avoid reentrancy when we're not using FBackgroundLogger as the frontend
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
        const FLogAllocator::FScope scopeAlloc;

        const size_t sizeInBytes = (sizeof(FDeferredLog) + text.SizeInBytes());
        auto* const log = INPLACE_NEW(scopeAlloc.Heap().Allocate(sizeInBytes), FDeferredLog)(scopeAlloc.Heap());
        log->LowLevelLogger = _userLogger;
        log->Category = &category;
        log->Level = level;
        log->Site = site;
        log->TextLength = checked_cast<u32>(text.size());
        log->Bucket = checked_cast<u32>(scopeAlloc.Index);
        log->AllocSizeInBytes = checked_cast<u32>(sizeInBytes);

        FPlatformMemory::Memcpy(log + 1, text.data(), text.SizeInBytes());

        TaskManager_().Run(MakeFunction(log, &FDeferredLog::Log));
    }

    virtual void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args) override final {
        const FLogAllocator::FScope scopeAlloc;

        MEMORYSTREAM_LINEARHEAP() buf(scopeAlloc.Heap());
        buf.reserve(sizeof(FDeferredLog) + format.SizeInBytes());
        buf.resize(sizeof(FDeferredLog)); // reserve space for FDeferredLog entry
        buf.SeekO(0, ESeekOrigin::End); // seek at the end of the stream
        {
            FWTextWriter oss(&buf);
            FormatArgs(oss, format, args);
        }
        auto* const log = INPLACE_NEW(buf.Pointer(), FDeferredLog)(scopeAlloc.Heap());

        size_t sizeInBytes = 0;
        const TMemoryView<u8> stolen = buf.StealDataUnsafe(log->get_allocator(), &sizeInBytes);
        Assert(stolen.data() == (u8*)log);

        log->LowLevelLogger = _userLogger;
        log->Category = &category;
        log->Level = level;
        log->Site = site;
        log->TextLength = checked_cast<u32>((sizeInBytes - sizeof(FDeferredLog)) / sizeof(wchar_t));
        log->Bucket = checked_cast<u32>(scopeAlloc.Index);
        log->AllocSizeInBytes = checked_cast<u32>(stolen.SizeInBytes());

        TaskManager_().Run(MakeFunction(log, &FDeferredLog::Log));
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
    }

private:
    FUserLogger* const _userLogger;

    FBackgroundLogger()
        : _userLogger(&FUserLogger::Get())
    {}

    static FTaskManager& TaskManager_() {
        return FBackgroundThreadPool::Get();
    }

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
    class ALIGN(16) FDeferredLog : public TLinearHeapAllocator<u8> {
    public:
        ILowLevelLogger* LowLevelLogger;
        const FCategory* Category;
        FSiteInfo Site;
        EVerbosity Level;

        u32 TextLength;

        u32 Bucket;
        u32 AllocSizeInBytes;

        FDeferredLog(FLinearHeap& heap) : TLinearHeapAllocator<u8>(heap) {}

        FDeferredLog(const FDeferredLog&) = delete;
        FDeferredLog& operator =(const FDeferredLog&) = delete;

        FWStringView Text() const { return { (const wchar_t *)(this + 1), TextLength }; }

        void Log(ITaskContext&) {
            const FSuicideScope_ logScope(this);
            LowLevelLogger->Log(*Category, Level, Site, Text());
        }

        TLinearHeapAllocator<u8>& get_allocator() { return *this; }
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
        }
        ~FSuicideScope_() {
            Heap().Release(Log, Log->AllocSizeInBytes);
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
        Format(oss, L"[{0}][{1:20}][{3:-8}][{2:-15}] ", site.Timestamp.ToDateTimeUTC(), FThreadContext::GetThreadName(site.ThreadId), category.Name, level);
#   else // only thread hash :
        Format(oss, L"[{0}][{1:#5}][{3:-8}][{2:-15}] ", site.Timestamp.ToDateTimeUTC(), FThreadContext::GetThreadHash(site.ThreadId), category.Name, level);
#   endif
#else
        Format(oss, L"[{0}][{2:-8}][{1:-15}] ", site.Timestamp.ToDateTimeUTC(), category.Name, level);
#endif
    }

    static void Footer(FWTextWriter& oss, const ILogger::FCategory& category, ILogger::EVerbosity level, const ILogger::FSiteInfo& site) {
#if PPE_DUMP_SITE_ON_LOG
        Format(oss, L"\n\tat {0}:{1}\n", site.Filename, site.Line);
#else
        NOOP(category, level, site);
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
        Assert(category.Verbosity & level);

        wchar_t tmp[8192];
        FWFixedSizeTextWriter oss(tmp);

        FLogFormat::Header(oss, category, level, site);
        oss.Write(text);
        FLogFormat::Footer(oss, category, level, site);

        oss << Eos;

        FPlatformDebug::OutputDebug(tmp);
    }

    virtual void LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args) override final {
        Assert(category.Verbosity & level);

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

    if (category.Verbosity & level)
        CurrentLogger_().Log(category, level, site, text);

    HandleFatalLogIFN_(level);
}
//----------------------------------------------------------------------------
void FLogger::LogArgs(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& format, const FWFormatArgList& args) {
    const FIsInLoggerScope loggerScope;

    if (category.Verbosity & level)
        CurrentLogger_().LogArgs(category, level, site, format, args);

    HandleFatalLogIFN_(level);
}
//----------------------------------------------------------------------------
void FLogger::Start() {
    FLogAllocator::Create();

    FUserLogger::Create();

    RegisterLogger(
        FCurrentProcess::Get().StartedWithDebugger()
            ? FLogger::MakeOutputDebug()
            : FLogger::MakeStdout() );

    RegisterLogger(FLogger::MakeRollFile(L"Saved:/Log/Core.log"));

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
class FFileStreamLogger_ final : public ILogger {
public:
    explicit FFileStreamLogger_(FFileStreamWriter&& ostream)
        : _ostream(std::move(ostream))
        , _buffered(&_ostream) {
        Assert(_ostream.Good());
    }

public: // ILogger
    virtual void Log(const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& text) override final {
        FWTextWriter oss(&_buffered);
        FLogFormat::Print(oss, category, level, site, text);
    }

    virtual void Flush(bool) override final {
        _buffered.Flush();
    }

private:
    FFileStreamWriter _ostream;
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
    FFileStreamWriter ostream(
        FFileStream::OpenWrite(filename,
            EAccessPolicy::Create|EAccessPolicy::Append|EAccessPolicy::Binary|EAccessPolicy::ShareRead) );
    AssertRelease(ostream.Good());
    return NEW_REF(Logger, FFileStreamLogger_)(std::move(ostream));
}
//----------------------------------------------------------------------------
PLogger FLogger::MakeRollFile(const wchar_t* filename) {
    Verify(FPlatformFile::RollFile(filename));
    return MakeAppendFile(filename);
}
//----------------------------------------------------------------------------
PLogger FLogger::MakeFunctor(TFunction<void(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&)>&& write) {
    Assert(write);
    return NEW_REF(Logger, FFunctorLogger_)(std::move(write));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, FLogger::EVerbosity level) {
    auto sep = Fmt::NotFirstTime('|');
    if (level & FLogger::EVerbosity::Debug)     oss << sep << "Debug";
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

#endif //!USE_DEBUG_LOGGER