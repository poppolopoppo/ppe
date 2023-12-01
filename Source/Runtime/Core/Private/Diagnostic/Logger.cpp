// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Diagnostic/Logger.h"

#if USE_PPE_LOGGER

#   include "Allocator/SlabHeap.h"
#   include "Allocator/SlabAllocator.h"
#   include "Container/SparseArray.h"
#   include "Container/Vector.h"

#   include "Diagnostic/CurrentProcess.h"
#   include "Diagnostic/IgnoreList.h"

#   include "HAL/PlatformConsole.h"
#   include "HAL/PlatformDebug.h"
#   include "HAL/PlatformFile.h"
#   include "HAL/PlatformMemory.h"

#   include "IO/BufferedStream.h"
#   include "IO/ConstChar.h"
#   include "IO/FileStream.h"
#   include "IO/FormatHelpers.h"
#   include "IO/StreamProvider.h"
#   include "IO/String.h"
#   include "IO/StringBuilder.h"
#   include "IO/StringView.h"
#   include "IO/TextWriter.h"

#   include "Memory/InSituPtr.h"
#   include "Memory/MemoryView.h"

#   include "Meta/Optional.h"
#   include "Meta/Singleton.h"
#   include "Meta/ThreadResource.h"
#   include "Meta/Utility.h"

#   include "Misc/Function.h"

#   include "Thread/AtomicSet.h"
#   include "Thread/ThreadSafe.h"
#   include "Thread/AtomicSpinLock.h"
#   include "Thread/Task/TaskManager.h"
#   include "Thread/Task/TaskHelpers.h"
#   include "Thread/ThreadContext.h"
#   include "Thread/ThreadPool.h"

#   include "Time/Timestamp.h"

#   include <atomic>
#   include <mutex>
#   include <iostream>

#   define PPE_DUMP_THREAD_ID              1
#   define PPE_DUMP_THREAD_NAME            0
#   define PPE_DUMP_SITE_ON_LOG            0
#   define PPE_DUMP_SITE_ON_ERROR          1

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, LogDefault)
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static ELoggerVerbosity GLoggerVerbosity_ = ELoggerVerbosity::All;
#if !USE_PPE_FINAL_RELEASE
static FLoggerCategory::EFlags GLoggerFlags_ = Default;
#endif
//----------------------------------------------------------------------------
static THREAD_LOCAL bool GIsInLogger_ = false;
struct FIsInLoggerScope {
    const bool WasInLogger;
    FIsInLoggerScope() : WasInLogger(GIsInLogger_) { GIsInLogger_ = true; }
    ~FIsInLoggerScope() { GIsInLogger_ = WasInLogger; }
};
//----------------------------------------------------------------------------
struct FLoggerTypes {
    using EVerbosity = ELoggerVerbosity;
    using FCategory = FLoggerCategory;
    using FSiteInfo = FLoggerSiteInfo;
};
//----------------------------------------------------------------------------
class ILowLevelLogger : public FLoggerTypes, public ILogger {
public:
    virtual void Log(const FCategory& category, FSiteInfo&& site, const FStringView& text) = 0;
    virtual void LogDirect(const FCategory& category, FSiteInfo&& site, const TFunction<void(FTextWriter&)>& direct) = 0;
    virtual void LogFmt(const FCategory& category, FSiteInfo&& site, const FStringView& format, const FFormatArgList& args) = 0;
    virtual void LogStructured(const FCategory& category, FSiteInfo&& site, const FStringView& text, Opaq::object_init&& object) = 0;
    virtual void Printf(const FCategory& category, FSiteInfo&& site, const FConstChar& format, va_list args) = 0;
    virtual void RecordArgs(const FCategory& category, FSiteInfo&& site, const FFormatArgList& record) = 0;

    virtual void OnRelease(ILowLevelLogger& newLogger) {
        Unused(newLogger);
        Flush(true);
    }
};
//----------------------------------------------------------------------------
// Use a custom allocator for logger to handle contention, fragmentation and get re-entrancy
class FLogAllocator : FLoggerTypes {
public:
    using heap_type = SLABHEAP(Logger);
    using allocator_type = SLAB_ALLOCATOR(Logger);

    struct CACHELINE_ALIGNED FBucket {
        TPtrRef<FLogAllocator> Owner;
        heap_type Heap;
        FBucket() NOEXCEPT {
            Heap.SetSlabSize(16_KiB);
        }
        ~FBucket() {
            Heap.ReleaseAll();
        }
    };

    struct FScope : Meta::FNonCopyable {
        TPtrRef<FBucket> Bucket;

        heap_type& Heap() const { return Bucket->Heap; }
        allocator_type Allocator() const { return { Bucket->Heap }; }
        operator allocator_type() const NOEXCEPT { return Allocator(); }

        explicit FScope(FLogAllocator& owner) NOEXCEPT
        :   Bucket(owner.NextFreeBucket_())
        {}

        explicit FScope(FBucket& bucket) NOEXCEPT
        :   Bucket(bucket.Owner->OpenBucket_(bucket))
        {}

        ~FScope() NOEXCEPT {
            Bucket->Owner->CloseBucket_(Bucket);
        }
    };

    struct FMessage {
        TPtrRef<FBucket> Bucket;
        FLoggerMessage Inner;
        void* BlockData{ nullptr };
        u32 BlockSizeInBytes{ 0 };
        i32 RefCount{ 0 };

        FAllocatorBlock Block() const { return { BlockData, BlockSizeInBytes }; }

        FMessage(FBucket& bucket, FAllocatorBlock block) NOEXCEPT
        :   FMessage(bucket, block, FLoggerMessage{})
        {}

        FMessage(FBucket& bucket, FAllocatorBlock block, FLoggerMessage&& msg) NOEXCEPT
        :   Bucket(bucket)
        ,   Inner(std::move(msg))
        ,   BlockData(block.Data)
        ,   BlockSizeInBytes(checked_cast<u32>(block.SizeInBytes))
        ,   RefCount(1)
        {}

        FMessage& Acquire() {
            Assert_NoAssume(RefCount > 0);
            ++RefCount;
            return (*this);
        }
        bool Release() {
            Assert(RefCount > 0);
            if (--RefCount == 0) {
                Bucket->Owner->DeallocateMessage(*this);
                return true;
            }
            return false;
        }
    };

    FLogAllocator() NOEXCEPT {
        for (FBucket& bucket : _buckets)
            bucket.Owner = this;
    }

    ~FLogAllocator() {
        Clear_ReleaseMemory();
    }

    TPtrRef<FMessage> AllocateLog(const FCategory& category, FSiteInfo&& site, const FStringView& text) {
        TPtrRef<FMessage> msg;
        {
            const FScope scopeLock(*this);
            const FAllocatorBlock block{ scopeLock.Allocator().Allocate(sizeof(FMessage)) };
            msg = INPLACE_NEW(block.Data, FMessage){ scopeLock.Bucket, block };
        }

        msg->Inner = {category, std::move(site), text.data(), false};
        return msg;
    }
    TPtrRef<FMessage> AllocateLogDirect(const FCategory& category, FSiteInfo&& site, const TFunction<void(FTextWriter&)>& direct) {
         const FScope scopeLock(*this);

        MEMORYSTREAM_SLAB(Logger) oss(scopeLock.Allocator());
        oss.Append(sizeof(FMessage));

        FTextWriter writer(oss);
        direct(writer);
        writer << Eos;
        const FStringView text = oss.MakeView().CutStartingAt(sizeof(FMessage)).Cast<const char>().ShiftBack(/* '\0' */);

        TPtrRef<FMessage> msg{ reinterpret_cast<FMessage*>(oss.data()) };
        INPLACE_NEW(msg.get(), FMessage){ scopeLock.Bucket, oss.StealDataUnsafe(),
            FLoggerMessage{category, std::move(site), text.data(), true} };
        return msg;
    }
    TPtrRef<FMessage> AllocateLogFmt(const FCategory& category, FSiteInfo&& site, const FStringView& format, const FFormatArgList& args) {
        const FScope scopeLock(*this);

        MEMORYSTREAM_SLAB(Logger) oss(scopeLock.Allocator());
        oss.reserve(sizeof(FMessage) + format.size());
        oss.Append(sizeof(FMessage));

        FTextWriter writer(oss);
        FormatArgs(writer, format, args);
        writer << Eos;
        const FStringView text = oss.MakeView().CutStartingAt(sizeof(FMessage)).Cast<const char>().ShiftBack(/* '\0' */);

        TPtrRef<FMessage> msg{ reinterpret_cast<FMessage*>(oss.data()) };
        INPLACE_NEW(msg.get(), FMessage){ scopeLock.Bucket, oss.StealDataUnsafe(),
            {category, std::move(site), text.data(), true} };
        return msg;
    }
    TPtrRef<FMessage> AllocateLogStructured(const FCategory& category, FSiteInfo&& site, const FStringView& text, Opaq::object_init&& object) {
        TPtrRef<FMessage> msg;
        {
            const size_t objectSize = BlockSize(object); // outside of allocator lock
            const FScope scopeLock(*this);
            const FAllocatorBlock block{ scopeLock.Allocator().Allocate(sizeof(FMessage) + objectSize) };
            msg = INPLACE_NEW(block.Data, FMessage){ scopeLock.Bucket, block };
        }

        const FAllocatorBlock dataBlock = FAllocatorBlock::From(msg->Block().MakeView().CutStartingAt(sizeof(FMessage)));
        msg->Inner = {category, std::move(site), text.data(), false,
            std::get<Opaq::object_view>(*NewBlock(dataBlock, object).Value()) };
        return msg;
    }
    TPtrRef<FMessage> AllocatePrintf(const FCategory& category, FSiteInfo&& site, const FConstChar& format, va_list args) {
        MALLOCA_POD(char, buf, 2048);
        const int len = FPlatformString::Printf(buf.data(), buf.Count, format, args);
        const FStringView written = buf.MakeView().CutBefore(len + 1/* null char */);

        TPtrRef<FMessage> msg;
        {
            const FScope scopeLock(*this);
            const FAllocatorBlock block{ scopeLock.Allocator().Allocate(sizeof(FMessage) + written.SizeInBytes()) };
            msg = INPLACE_NEW(block.Data, FMessage){ scopeLock.Bucket, block };
        }

        const TMemoryView<char> text = msg->Block().MakeView().CutStartingAt(sizeof(FMessage)).Cast<char>();
        Unused(Copy(text, written));
        msg->Inner = {category, std::move(site), text.data(), true};
        return msg;
    }
    TPtrRef<FMessage> AllocateRecordArgs(const FCategory& category, FSiteInfo&& site, const FFormatArgList& record) {
        const FScope scopeLock(*this);

        MEMORYSTREAM_SLAB(Logger) oss(scopeLock.Allocator());
        oss.reserve(sizeof(FMessage) + record.size() * 8 * sizeof(char));
        oss.Append(sizeof(FMessage));

        FTextWriter writer(oss);
        FormatRecord(writer, record);
        writer << Eos;
        const FStringView text = oss.MakeView().CutStartingAt(sizeof(FMessage)).Cast<const char>().ShiftBack(/* '\0' */);

        TPtrRef<FMessage> msg{ reinterpret_cast<FMessage*>(oss.data()) };
        INPLACE_NEW(msg.get(), FMessage){ scopeLock.Bucket, oss.StealDataUnsafe(),
            {category, std::move(site), text.data(), true} };
        return msg;
    }

    void DeallocateMessage(FMessage& msg) const {
        if (Likely(msg.BlockData)) {
            Assert_NoAssume(msg.Bucket->Owner.get() == this);
            const FScope scopeLock(msg.Bucket);
            msg.Bucket->Heap.Deallocate(msg.BlockData, msg.BlockSizeInBytes);
        }
    }

    void TrimCache() {
        forrange(i, 0, NumAllocationBuckets) {
            if (_freeBuckets.AcquireIFP(i)) {
                _buckets[i].Heap.TrimMemory();
                _freeBuckets.NotifyDeallocate(i);
            }
        }
    }

    void Clear_ReleaseMemory() {
        // acquire all buckets when destructing:
        // - checks that no asynchronous task is still consuming the buckets
        // - checks that no message was leaked
        i32 backoff = 0;
        forrange(i, 0, NumAllocationBuckets) {
            for (;;) {
                if (_freeBuckets.AcquireIFP(i))
                    break;
                FPlatformProcess::SleepForSpinning(backoff);
            }
        }
        // force release all memory
        for (FBucket& bucket : _buckets) {
            bucket.Heap.ReleaseAll();
        }
        // release free buckets
        _freeBuckets.ResetMask(MaskAllocationBuckets);
        details::NotifyAllAtomicBarrier(&_freeBuckets.Data());
    }

private:
    STATIC_CONST_INTEGRAL(u32, NumAllocationBuckets, 8);
    STATIC_CONST_INTEGRAL(TBitMask<u32>, MaskAllocationBuckets, (1_u32 << NumAllocationBuckets) - 1_u32);
    STATIC_ASSERT(Meta::IsPow2(NumAllocationBuckets));

    FBucket _buckets[NumAllocationBuckets];
    TAtomicBitMask<u32> _freeBuckets{ MaskAllocationBuckets };
    u8 _revision{ 0 };

    FORCE_INLINE FBucket& OpenBucket_(FBucket& bucket) {
        const u32 index = checked_cast<u32>(&bucket - _buckets);
        _freeBuckets.WaitAcquire(index);
        return bucket;
    }
    FORCE_INLINE void CloseBucket_(FBucket& bucket) {
        const u32 index = checked_cast<u32>(&bucket - _buckets);
        _freeBuckets.NotifyDeallocate(index);
    }
    FORCE_INLINE FBucket& NextFreeBucket_() {
        const u32 index = _freeBuckets.WaitAllocateRoll(++_revision % NumAllocationBuckets);
        return _buckets[index];
    }
};
//----------------------------------------------------------------------------
// Composite for all loggers supplied through public API
class FUserLogger final : Meta::TStaticSingleton<FUserLogger>, public ILogger {
    friend Meta::TStaticSingleton<FUserLogger>;
    using singleton_type = Meta::TStaticSingleton<FUserLogger>;
public:
    using singleton_type::Create;
    using singleton_type::Destroy;
    using singleton_type::Get;

    ~FUserLogger() override {
        for (const Meta::TPointerWFlags<ILogger>& logger : _loggers) {
            AssertRelease_NoAssume(logger.Flag0());
            tracking_delete(logger.Get());
        }

        _loggers.clear_ReleaseMemory();
    }

    void Add(TPtrRef<ILogger> logger, bool autoDelete) {
        Assert(logger);

        const Meta::FLockGuard scopeLock(_barrier);
        Assert(not Contains(_loggers, logger.get()));

        Meta::TPointerWFlags<ILogger> registeredLogger;
        registeredLogger.Reset(logger, autoDelete, false);
        _loggers.push_back(std::move(registeredLogger));;
    }

    void Remove(TPtrRef<ILogger> logger) {
        Assert(logger);

        const Meta::FLockGuard scopeLock(_barrier);
        const auto it = std::find(_loggers.begin(), _loggers.end(), logger.get());

        Assert(_loggers.end() != it);
        _loggers.erase(it);
    }

public: // ILowLevelLogger
    virtual void LogMessage(const FLoggerMessage& msg) override final {
        const FReentrancyProtection_ scopeReentrant;
        if (scopeReentrant.WasLocked)
            return;

        const Meta::FLockGuard scopeLock(_barrier);
        for (const Meta::TPointerWFlags<ILogger>& logger : _loggers)
            logger->LogMessage(msg);
    }

    virtual void Flush(bool synchronous) override final {
        const FReentrancyProtection_ scopeReentrant;
        if (scopeReentrant.WasLocked)
            return;

        const Meta::FLockGuard scopeLock(_barrier);
        for (const Meta::TPointerWFlags<ILogger>& logger : _loggers)
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

    FUserLogger() = default; // private ctor

    std::mutex _barrier;
    VECTORINSITU(Logger, Meta::TPointerWFlags<ILogger>, 8) _loggers;
};
THREAD_LOCAL bool FUserLogger::FReentrancyProtection_::GLockTLS{ false };
//----------------------------------------------------------------------------
// Asynchronous logger used during the game
class FBackgroundLogger final : Meta::TStaticSingleton<FBackgroundLogger>, public ILowLevelLogger, FLogAllocator {
    friend Meta::TStaticSingleton<FBackgroundLogger>;
    using singleton_type = Meta::TStaticSingleton<FBackgroundLogger>;
public:
    using singleton_type::Create;
    using singleton_type::Destroy;
    using singleton_type::Get;

    void SetAsynchronous(bool enabled) {
        _enableAsynchronous = enabled;
    }

public: // ILowLevelLogger
    virtual void Log(const FCategory& category, FSiteInfo&& site, const FStringView& text) override final {
        LogMessageBackground_(AllocateLog(category, std::move(site), text));
    }
    virtual void LogDirect(const FCategory& category, FSiteInfo&& site, const TFunction<void(FTextWriter&)>& direct) override final {
        LogMessageBackground_(AllocateLogDirect(category, std::move(site), direct));
    }
    virtual void LogFmt(const FCategory& category, FSiteInfo&& site, const FStringView& format, const FFormatArgList& args) override final {
        LogMessageBackground_(AllocateLogFmt(category, std::move(site), format, args));
    }
    virtual void LogStructured(const FCategory& category, FSiteInfo&& site, const FStringView& text, Opaq::object_init&& object) override final {
        LogMessageBackground_(AllocateLogStructured(category, std::move(site), text, std::move(object)));
    }
    virtual void Printf(const FCategory& category, FSiteInfo&& site, const FConstChar& format, va_list args) override final {
        LogMessageBackground_(AllocatePrintf(category, std::move(site), format , args));
    }
    virtual void RecordArgs(const FCategory& category, FSiteInfo&& site, const FFormatArgList& record) override final {
        LogMessageBackground_(AllocateRecordArgs(category, std::move(site), record));
    }

    virtual void LogMessage(const FLoggerMessage& msg) override final {
        // this path is always synchronous
        _userLogger->LogMessage(msg);
    }

    virtual void Flush(bool synchronous) override final {
        const bool enableAsynchronous = _enableAsynchronous.load(std::memory_order_relaxed);
        if ((not enableAsynchronous || synchronous) && not GIsInLogger_) {
            // wait for all potential logs before flushing
            for (u32 loop = 0; loop < 5 && TaskManager_().WaitForAll(500/* 0.5 seconds */); ++loop){}
            _userLogger->Flush(true);
        }
        else if (enableAsynchronous) {
            TaskManager_().RunAndWaitFor([this](ITaskContext&) {
                _userLogger->Flush(true); // flush synchronously in asynchronous task
                // good idea to trim the linear heaps when flushing
                TrimCache();
            },  ETaskPriority::Low );
        }
    }

private:
    const TPtrRef<FUserLogger> _userLogger;
    std::atomic_bool _enableAsynchronous{ true };

    FBackgroundLogger() NOEXCEPT
    :   FLogAllocator()
    ,   _userLogger(FUserLogger::Get())
    {}

    ~FBackgroundLogger() override {
        TaskManager_().WaitForAll(); // blocking wait before destroying, avoid necrophilia
    }

    static FTaskManager& TaskManager_() {
        return FBackgroundThreadPool::Get();
    }

    bool ShouldTreatInBackground_(const FLoggerCategory& category, EVerbosity level) const NOEXCEPT {
        // background logger can be disabled
        if (not _enableAsynchronous.load(std::memory_order_relaxed))
            return false;

        // don't treat errors in background, log category flags can also force immediate mode
        return (not (category.Flags & FLoggerCategory::Immediate || level ^ (EVerbosity::Fatal|EVerbosity::Error)) );
    }

    FORCE_INLINE void LogMessageBackground_(FMessage& msg) const {
        Assert_NoAssume(msg.RefCount > 0);
        if (Likely(ShouldTreatInBackground_(msg.Inner.Category, msg.Inner.Level())))
            return TaskManager_().Run([this, msg{MakePtrRef(msg)}](ITaskContext&) {
                LogMessageImmediate_(msg);
            });
        LogMessageImmediate_(msg);
    }

    NO_INLINE void LogMessageImmediate_(FMessage& msg) const {
        Assert_NoAssume(msg.RefCount > 0);
        DEFERRED{ msg.Release(); };
        _userLogger->LogMessage(msg.Inner);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FLogFmt : FLoggerTypes {
public:
    static FTimepoint StartedAt() {
        ONE_TIME_INITIALIZE(const FTimepoint, GStartedAt, FTimepoint::Now());
        return GStartedAt;
    }

    static void Header(FTextWriter& oss, const FCategory& category, const FLogger::FSiteInfo& site) {
        const FSeconds elapsed = site.LogTime.ElapsedSince(StartedAt());
#if PPE_DUMP_THREAD_ID
#   if PPE_DUMP_THREAD_NAME
        Format(oss, _PPE_LOG_STRING("[{0:#-10f4}][{1:20}][{3:-9}][{2}] ") elapsed.Value(), FThreadContext::GetThreadName(site.ThreadId), category.Name, site.Level());
#   else // only thread hash :
        Format(oss, _PPE_LOG_STRING("[{0:#-10f4}][{1:#5}][{3:-9}][{2}] "), elapsed.Value(), FThreadContext::GetThreadHash(site.ThreadId), category.Name, site.Level());
#   endif
#else
        Format(oss, _PPE_LOG_STRING("[{0:#-10f4}][{2:-9}][{1}] "), elapsed.Value(), category.Name, site.Level());
#endif
    }

    static void Footer(FTextWriter& oss, const FCategory&, const FLogger::FSiteInfo& site) {
#if PPE_DUMP_SITE_ON_LOG
        Format(oss, _PPE_LOG_STRING("\n\tat {0}:{1}\n"), site.Filename, site.Line);
#elif PPE_DUMP_SITE_ON_ERROR
        if (Unlikely(site.Level() & (ELoggerVerbosity::Error|ELoggerVerbosity::Fatal)))
            Format(oss, _PPE_LOG_STRING("\n\tat {0}:{1}"), site.SourceFile, site.SourceFile);
        oss << Eol;
#else
        Unused(site);
        oss << Eol;
#endif
    }

    static void Print(FTextWriter& oss, const FCategory& category, const FLogger::FSiteInfo& site, const FStringView& text, const TPtrRef<const Opaq::object_view>& data) {
        Header(oss, category, site);
        oss.Write(text);
        if (data) {
            Assert_NoAssume(not data->empty());
            oss << ' ' << FTextFormat::Compact << data;
        }
        Footer(oss, category, site);
    }
};
//----------------------------------------------------------------------------
// Used before & after main when no debugger is attached, ignores everything
class FDevNullLogger final : public ILowLevelLogger {
public:
    static ILowLevelLogger* Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FDevNullLogger, GInstance);
        return (&GInstance);
    }

public: // ILowLevelLogger
    virtual void Log(const FCategory& category, FSiteInfo&& site, const FStringView& text) override final { Unused(category, site, text); }
    virtual void LogDirect(const FCategory& category, FSiteInfo&& site, const TFunction<void(FTextWriter&)>& direct) override final { Unused(category, site, direct); }
    virtual void LogFmt(const FCategory& category, FSiteInfo&& site, const FStringView& format, const FFormatArgList& args) override final { Unused(category, site, format, args); }
    virtual void LogStructured(const FCategory& category, FSiteInfo&& site, const FStringView& text, Opaq::object_init&& object) override final { Unused(category, site, text, object); }
    virtual void Printf(const FCategory& category, FSiteInfo&& site, const FConstChar& format, va_list args) override final { Unused(category, site, format, args); }
    virtual void RecordArgs(const FCategory& category, FSiteInfo&& site, const FFormatArgList& record) override final { Unused(category, site, record); }

    virtual void LogMessage(const FLoggerMessage& msg) override final { Unused(msg); }
    virtual void Flush(bool synchronous) override final { Unused(synchronous); }
};
//----------------------------------------------------------------------------
// Used before main when logger is not yet created
class FAccumulatingLogger final : public ILowLevelLogger, Meta::TStaticSingleton<FAccumulatingLogger>, FLogAllocator {
    friend Meta::TStaticSingleton<FAccumulatingLogger>;
public:
    using singleton_type = Meta::TStaticSingleton<FAccumulatingLogger>;
    using singleton_type::Get;

    static void Create() {
        singleton_type::Create();
    }

    ~FAccumulatingLogger() override {
        FAccumulatingLogger::Flush(true);
    }

    void AddToHistory(FMessage& msg) {
        _history.LockExclusive()->Emplace(msg);
    }

    void FlushAccumulatedLogs(ILowLevelLogger& other) {
        Assert(this != &other);

        const auto exclusiveHist = _history.LockExclusive();
        for (const TPtrRef<FMessage>& msg : *exclusiveHist) {
            other.LogMessage(msg->Inner);
        #if 0 // release everything outside of the loop
            msg->Release();
        #else
            Assert_NoAssume(msg->RefCount == 1);
        #endif
        }

        exclusiveHist->Clear_ReleaseMemory();
        FLogAllocator::Clear_ReleaseMemory();
    }

public:
    virtual void Log(const FCategory& category, FSiteInfo&& site, const FStringView& text) override final {
        AddToHistory(AllocateLog(category, std::move(site), text));
    }
    virtual void LogDirect(const FCategory& category, FSiteInfo&& site, const TFunction<void(FTextWriter&)>& direct) override final {
        AddToHistory(AllocateLogDirect(category, std::move(site), direct));
    }
    virtual void LogFmt(const FCategory& category, FSiteInfo&& site, const FStringView& format, const FFormatArgList& args) override final {
        AddToHistory(AllocateLogFmt(category, std::move(site), format, args));
    }
    virtual void LogStructured(const FCategory& category, FSiteInfo&& site, const FStringView& text, Opaq::object_init&& object) override final {
        AddToHistory(AllocateLogStructured(category, std::move(site), text, std::move(object)));
    }
    virtual void Printf(const FCategory& category, FSiteInfo&& site, const FConstChar& format, va_list args) override final {
        AddToHistory(AllocatePrintf(category, std::move(site), format, args));
    }
    virtual void RecordArgs(const FCategory& category, FSiteInfo&& site, const FFormatArgList& record) override final {
        AddToHistory(AllocateRecordArgs(category, std::move(site), record));
    }

    virtual void LogMessage(const FLoggerMessage& msg) override final {
        Unused(msg);
        AssertNotReached(); // should not dispatch messages, only saves them for later
    }

    virtual void Flush(bool) override final
    {}

    virtual void OnRelease(ILowLevelLogger& newLogger) override final {
        ILowLevelLogger::OnRelease(newLogger);

        if (this != &newLogger)
            FlushAccumulatedLogs(newLogger);

        singleton_type::Destroy(); // /!\ suicide this
    }

private:
    using history_type = SPARSEARRAY_INSITU(Logger, TPtrRef<FLogAllocator::FMessage>);
    TThreadSafe<history_type, EThreadBarrier::CriticalSection> _history;
};
//----------------------------------------------------------------------------
// Used before & after main when debugger attached, no dependencies on allocators, always immediate
#if USE_PPE_PLATFORM_DEBUG
class FDebuggingLogger final : public ILowLevelLogger {
public:
    static TPtrRef<ILowLevelLogger> Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FDebuggingLogger, GInstance);
        return GInstance;
    }

public: // ILowLevelLogger
    using FTransientMemoryStream_ = TMemoryStream<INLINE_ALLOCATOR(Logger, u8, 4096)>;

    virtual void Log(const FCategory& category, FSiteInfo&& site, const FStringView& text) override final {
        Assert_NoAssume(category.Verbosity ^ site.Level());

        FTransientMemoryStream_ tmp;
        FTextWriter oss(&tmp);

        FLogFmt::Header(oss, category, site);
        oss.Write(text);
        FLogFmt::Footer(oss, category, site);
        oss << Eos;

        FPlatformDebug::OutputDebug(tmp.MakeView().Cast<char>().data());
    }

    virtual void LogDirect(const FCategory& category, FSiteInfo&& site, const TFunction<void(FTextWriter&)>& direct) override final {
        Assert_NoAssume(category.Verbosity ^ site.Level());

        FTransientMemoryStream_ tmp;
        FTextWriter oss(&tmp);

        FLogFmt::Header(oss, category, site);
        direct(oss);
        FLogFmt::Footer(oss, category, site);
        oss << Eos;

        FPlatformDebug::OutputDebug(tmp.MakeView().Cast<char>().data());
    }

    virtual void LogFmt(const FCategory& category, FSiteInfo&& site, const FStringView& format, const FFormatArgList& args) override final {
        Assert_NoAssume(category.Verbosity ^ site.Level());

        FTransientMemoryStream_ tmp;
        FTextWriter oss(&tmp);

        FLogFmt::Header(oss, category, site);
        FormatArgs(oss, format, args);
        FLogFmt::Footer(oss, category, site);
        oss << Eos;

        FPlatformDebug::OutputDebug(tmp.MakeView().Cast<char>().data());
    }

    virtual void LogStructured(const FCategory& category, FSiteInfo&& site, const FStringView& text, Opaq::object_init&& object) override final {
        Assert_NoAssume(category.Verbosity ^ site.Level());

        FTransientMemoryStream_ tmp;
        FTextWriter oss(&tmp);

        FLogFmt::Header(oss, category, site);
        oss << text << ' ' << FTextFormat::Compact << object;
        FLogFmt::Footer(oss, category, site);
        oss << Eos;

        FPlatformDebug::OutputDebug(tmp.MakeView().Cast<char>().data());
    }

    virtual void Printf(const FCategory& category, FSiteInfo&& site, const FConstChar& format, va_list args) override final {
        Assert_NoAssume(category.Verbosity ^ site.Level());

        char tmpStringBuf[2048];
        const int len = FPlatformString::Printf(tmpStringBuf, lengthof(tmpStringBuf), format, args);

        FTransientMemoryStream_ tmp;
        FTextWriter oss(&tmp);

        FLogFmt::Header(oss, category, site);
        oss << FStringView(tmpStringBuf, len);
        FLogFmt::Footer(oss, category, site);
        oss << Eos;

        FPlatformDebug::OutputDebug(tmp.MakeView().Cast<char>().data());
    }

    virtual void RecordArgs(const FCategory& category, FSiteInfo&& site, const FFormatArgList& record) override final {
        Assert_NoAssume(category.Verbosity ^ site.Level());

        FTransientMemoryStream_ tmp;
        FTextWriter oss(&tmp);

        FLogFmt::Header(oss, category, site);
        FormatRecord(oss, record);
        FLogFmt::Footer(oss, category, site);
        oss << Eos;

        FPlatformDebug::OutputDebug(tmp.MakeView().Cast<char>().data());
    }

    virtual void LogMessage(const FLoggerMessage& msg) override final {
        Assert_NoAssume(msg.Category->Verbosity ^ msg.Level());

        FTransientMemoryStream_ tmp;
        FTextWriter oss(&tmp);

        FLogFmt::Header(oss, msg.Category, msg.Site);
        oss << msg.Text();
        if (msg.Data)
            oss << ' ' << msg.Data;
        FLogFmt::Footer(oss, msg.Category, msg.Site);
        oss << Eos;

        FPlatformDebug::OutputDebug(tmp.MakeView().Cast<char>().data());
    }

    virtual void Flush(bool) override final {} // always synchronous => no need to flush
};
#endif //!USE_PPE_PLATFORM_DEBUG
//----------------------------------------------------------------------------
struct FLowLevelLogger {
    static TPtrRef<ILowLevelLogger> BeforeMain() {
#if USE_PPE_PLATFORM_DEBUG
        if (FPlatformDebug::IsDebuggerPresent())
            return FDebuggingLogger::Get();
#endif

        FAccumulatingLogger::Create();
        return std::addressof(FAccumulatingLogger::Get());
    }

    static TPtrRef<ILowLevelLogger> AfterMain() {
#if USE_PPE_PLATFORM_DEBUG
        if (FPlatformDebug::IsDebuggerPresent())
            return FDebuggingLogger::Get();
#endif

        return FDevNullLogger::Get();
    }

    static TThreadSafe<TPtrRef<ILowLevelLogger>, EThreadBarrier::RWDataRaceCheck>& InternalGet_() {
        static TThreadSafe<TPtrRef<ILowLevelLogger>, EThreadBarrier::RWDataRaceCheck> GLowerLevelLogger_{ BeforeMain() };
        return GLowerLevelLogger_;
    }

    static void Setup(ILowLevelLogger& pimpl) {
        TPtrRef<ILowLevelLogger> logger{ pimpl };
        swap(InternalGet_().LockExclusive().Value(), logger);
        if (logger)
            logger->OnRelease(pimpl);
    }

    static ILowLevelLogger& Get() NOEXCEPT {
        return InternalGet_().LockShared().Value();
    }
};
//----------------------------------------------------------------------------
FORCE_INLINE static void HandleFatalLogIFN_(FLogger::EVerbosity level) {
    if (Unlikely(FLogger::EVerbosity::Fatal == level)) {
        FLowLevelLogger::Get().Flush(true);
        AssertNotReached();
    }
}
//----------------------------------------------------------------------------
NODISCARD static bool NotifyLoggerMessage_(
    const FLoggerCategory& category,
    const FLogger::FSiteInfo& site ) NOEXCEPT {
    Unused(site);
#if !USE_PPE_FINAL_RELEASE
    const bool breakOnError = (site.Level() == ELoggerVerbosity::Error) && (
        (category.Flags & FLoggerCategory::BreakOnError) ||
         (GLoggerFlags_ & FLoggerCategory::BreakOnError) );

    const bool breakOnWarning = (site.Level() == ELoggerVerbosity::Warning) && (
        (category.Flags & FLoggerCategory::BreakOnWarning) ||
         (GLoggerFlags_ & FLoggerCategory::BreakOnWarning) );

    if (Unlikely(breakOnError || breakOnWarning)) {
#   if USE_PPE_IGNORELIST
        FIgnoreList::FIgnoreKey ignoreKey;
        ignoreKey << MakeStringView("NotifyLoggerMessage")
                  << category.Name
                  << site.SourceFile.MakeView()
                  << MakeRawConstView(site.SourceLine);
        if (FIgnoreList::HitIFP(ignoreKey) == 0)
#   endif
        {
            PPE_DEBUG_BREAK();
        }
    }
#endif
    return (category.Verbosity ^ site.Level()) && (GLoggerVerbosity_ ^ site.Level());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLogger::Log(const FCategory& category, FSiteInfo&& site, const FStringView& text) {
    const FIsInLoggerScope loggerScope;

    if (NotifyLoggerMessage_(category, site))
        FLowLevelLogger::Get().Log(category, std::move(site), text);

    HandleFatalLogIFN_(site.Level());
}
//----------------------------------------------------------------------------
void FLogger::LogDirect(const FCategory& category, FSiteInfo&& site, const TFunction<void(FTextWriter&)>& direct) {
    const FIsInLoggerScope loggerScope;

    if (NotifyLoggerMessage_(category, site))
        FLowLevelLogger::Get().LogDirect(category, std::move(site), direct);

    HandleFatalLogIFN_(site.Level());
}
//----------------------------------------------------------------------------
void FLogger::LogFmt(const FCategory& category, FSiteInfo&& site, const FStringView& format, const FFormatArgList& args) {
    const FIsInLoggerScope loggerScope;

    if (NotifyLoggerMessage_(category, site))
        FLowLevelLogger::Get().LogFmt(category, std::move(site), format, args);

    HandleFatalLogIFN_(site.Level());
}
//----------------------------------------------------------------------------
void FLogger::LogStructured(const FCategory& category, FSiteInfo&& site, const FStringView& text, Opaq::object_init&& object) {
    const FIsInLoggerScope loggerScope;

    if (NotifyLoggerMessage_(category, site))
        FLowLevelLogger::Get().LogStructured(category, std::move(site), text, std::move(object));

    HandleFatalLogIFN_(site.Level());
}
//----------------------------------------------------------------------------
void FLogger::Printf(const FCategory& category, FSiteInfo&& site, const FConstChar& format, va_list args) {
    const FIsInLoggerScope loggerScope;

    if (NotifyLoggerMessage_(category, site))
        FLowLevelLogger::Get().Printf(category, std::move(site), format, args);

    HandleFatalLogIFN_(site.Level());
}
//----------------------------------------------------------------------------
void FLogger::RecordArgs(const FCategory& category, FSiteInfo&& site, const FFormatArgList& record) {
    const FIsInLoggerScope loggerScope;

    if (NotifyLoggerMessage_(category, site))
        FLowLevelLogger::Get().RecordArgs(category, std::move(site), record);

    HandleFatalLogIFN_(site.Level());
}
//----------------------------------------------------------------------------
void FLogger::Start() {
    FUserLogger::Create();

    // always create the system trace
    RegisterSystemTraceLogger();

    // don't create a log file when running with an attached debugger
    if (FCurrentProcess::StartedWithDebugger()) {
        RegisterOutputDebugLogger();

#if !USE_PPE_FINAL_RELEASE
        // break on warnings/errors when debugger is attached
        if (FCurrentProcess::Get().HasArgument(L"-LOGBreakOnError"))
            GLoggerFlags_ |= FLoggerCategory::BreakOnError;
        if (FCurrentProcess::Get().HasArgument(L"-LOGBreakOnWarning"))
            GLoggerFlags_ |= FLoggerCategory::BreakOnWarning;
#endif
    }
    else {
        const auto& proc = FCurrentProcess::Get();

        RegisterStdoutLogger();

        const FWString logPath = FPlatformFile::JoinPath({
            proc.SavedPath(), L"Log",
            MakeStringView(WSTRINGIZE(BUILD_FAMILY)) });

        VerifyRelease(FPlatformFile::CreateDirectoryRecursively(*logPath, nullptr));

        // output log as json by default, except if -LOGTxt was passed on the command-line
        bool useStructuredLogging = true;
        FWStringView extname = L".json";
        if (FCurrentProcess::Get().HasArgument(L"-LOGTxt")) {
            useStructuredLogging = false;
            extname = L".txt";
        }

        const FWString logFile = FPlatformFile::JoinPath({
            logPath, proc.ExecutableName() + extname });

        if (useStructuredLogging)
            RegisterRollJsonLogger(*logFile);
        else
            RegisterRollFileLogger(*logFile);
    }

    FBackgroundLogger::Create();
    FBackgroundLogger& backgroundLogger = FBackgroundLogger::Get();

    IF_CONSTEXPR(USE_PPE_MEMORY_DEBUGGING)
        backgroundLogger.SetAsynchronous(false);

    if (FCurrentProcess::Get().HasArgument(L"-LOGAsync"))
        backgroundLogger.SetAsynchronous(true);
    if (FCurrentProcess::Get().HasArgument(L"-LOGNoAsync"))
        backgroundLogger.SetAsynchronous(false);

    FLowLevelLogger::Setup(backgroundLogger);
}
//----------------------------------------------------------------------------
void FLogger::Shutdown() {
    FLowLevelLogger::Setup(FLowLevelLogger::AfterMain());

    FBackgroundLogger::Destroy();
    FUserLogger::Destroy();
}
//----------------------------------------------------------------------------
void FLogger::Flush(bool synchronous/* = true */) {
    FLowLevelLogger::Get().Flush(synchronous);
}
//----------------------------------------------------------------------------
void FLogger::RegisterLogger(TPtrRef<ILogger> logger, bool autoDelete) {
    if (logger)
        FUserLogger::Get().Add(logger, autoDelete);
}
//----------------------------------------------------------------------------
void FLogger::UnregisterLogger(TPtrRef<ILogger> logger) {
    Assert(logger);

    FUserLogger::Get().Remove(logger);
}
//----------------------------------------------------------------------------
ELoggerVerbosity FLogger::GlobalVerbosity() {
    return GLoggerVerbosity_;
}
//----------------------------------------------------------------------------
void FLogger::SetGlobalVerbosity(ELoggerVerbosity verbosity) {
    GLoggerVerbosity_ = verbosity;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_PLATFORM_DEBUG
class FOutputDebugLogger_ final : public ILogger {
public:
    virtual void LogMessage(const FLoggerMessage& msg) override final {
        DEFERRED{ _sb.clear(); };

        FLogFmt::Print(_sb, msg.Category, msg.Site, msg.Text().MakeView(), msg.Data);
        _sb << Eos;

        FPlatformDebug::OutputDebug(_sb.Written().data());
    }

    virtual void Flush(bool) override final {} // always synced

private:
    FStringBuilder _sb;
};
#endif //!USE_PPE_PLATFORM_DEBUG
//----------------------------------------------------------------------------
class FFileHandleLogger_ final : public ILogger {
public:
    explicit FFileHandleLogger_(FPlatformLowLevelIO::FHandle fileHandle, bool autoClose = true)
    :   _fileWriter(fileHandle)
    ,   _buffered(&_fileWriter, FPlatformMemory::PageSize)
    ,   _autoClose(autoClose)
    {}

    ~FFileHandleLogger_() override {
        if (_autoClose)
            Verify(FPlatformLowLevelIO::Close(_fileWriter.Handle()));
    }

    virtual void LogMessage(const FLoggerMessage& msg) override final {
        FTextWriter oss(_buffered);
        FLogFmt::Print(oss, msg.Category, msg.Site, msg.Text().MakeView(), msg.Data);
        oss << Eol;
    }

    virtual void Flush(bool) override final {
        _buffered.Flush();
    }

private:
    FFileStreamWriter _fileWriter;
    FBufferedStreamWriter _buffered;
    const bool _autoClose{ false };
};
//----------------------------------------------------------------------------
class FJsonHandleLogger_ final : public ILogger {
public:
    explicit FJsonHandleLogger_ (FPlatformLowLevelIO::FHandle fileHandle, bool autoClose = true)
    :   _fileWriter(fileHandle)
    ,   _buffered(&_fileWriter, FPlatformMemory::PageSize)
    ,   _autoClose(autoClose)
    {}

    ~FJsonHandleLogger_() override {
        if (_autoClose)
            Verify(FPlatformLowLevelIO::Close(_fileWriter.Handle()));
    }

    virtual void LogMessage(const FLoggerMessage& msg) override final {
        FTextWriter oss(_buffered);
        oss << FTextFormat::Compact << Opaq::object_init{
            {"timestamp", msg.Site.LogTime.Timestamp().Value() },
            {"tid", bit_cast<u32>(msg.Site.ThreadId)},
            {"category", msg.Category->Name},
            {"severity", static_cast<u32>(msg.Level())},
            {"message", msg.Text().MakeView()},
            {"data", msg.Data},
/*            {"site", Opaq::object_init{
                {"file", msg.Site.SourceFile},
                {"line", msg.Site.SourceLine}
            }}*/
        } << Eol;
    }

    virtual void Flush(bool) override final {
        _buffered.Flush();
    }

private:
    FFileStreamWriter _fileWriter;
    FBufferedStreamWriter _buffered;
    const bool _autoClose{ false };
};
//----------------------------------------------------------------------------
class FConsoleWriterLogger_ final : public ILogger {
    const bool _available;
public:
    FConsoleWriterLogger_()
    :   _available(FPlatformConsole::Open())
    {}

    ~FConsoleWriterLogger_() override {
        FPlatformConsole::Close();
    }

public: // ILogger
    virtual void LogMessage(const FLoggerMessage& msg) override final {
        if (not _available)
            return;

        DEFERRED{ _sb.clear(); };
        FLogFmt::Print(_sb, msg.Category, msg.Site, msg.Text().MakeView(), msg.Data);

        FPlatformConsole::EAttribute attrs{};

        switch (msg.Level()) {
        case ELoggerVerbosity::Info:
            attrs = FPlatformConsole::WHITE_ON_BLACK;
            break;
        case ELoggerVerbosity::Profiling:
            attrs = (FPlatformConsole::FG_MAGENTA | FPlatformConsole::BG_BLACK | FPlatformConsole::FG_INTENSITY);
            break;
        case ELoggerVerbosity::Emphasis:
            attrs = (FPlatformConsole::FG_WHITE | FPlatformConsole::BG_BLUE | FPlatformConsole::FG_INTENSITY);
            break;
        case ELoggerVerbosity::Warning:
            attrs = (FPlatformConsole::FG_YELLOW | FPlatformConsole::BG_BLACK | FPlatformConsole::FG_INTENSITY);
            break;
        case ELoggerVerbosity::Error:
            attrs = (FPlatformConsole::FG_RED | FPlatformConsole::BG_BLACK);
            break;
        case ELoggerVerbosity::Verbose:
            attrs = (FPlatformConsole::FG_GREEN | FPlatformConsole::BG_BLACK);
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

        FPlatformConsole::SyntaxicHighlight(_sb.Written(), attrs);
    }

    virtual void Flush(bool) override final {
        if (_available)
            FPlatformConsole::Flush();
    }

private:
    FStringBuilder _sb;
};
//----------------------------------------------------------------------------
#if USE_PPE_PLATFORM_DEBUG
class FSystemTraceLogger_ final : public ILogger {
public:
    virtual void LogMessage(const FLoggerMessage& msg) override final {
        const FTimestamp date = FTimestamp::Now();

        switch (msg.Level()) {
        case ELoggerVerbosity::Debug:
        case ELoggerVerbosity::Verbose:
            FPlatformDebug::TraceVerbose(msg.Site.ThreadId, msg.Category->Name.data(), date.Value(), msg.Site.SourceFile, msg.Site.SourceLine, msg.Text());
            break;

        case ELoggerVerbosity::Info:
        case ELoggerVerbosity::Profiling:
        case ELoggerVerbosity::Emphasis:
            FPlatformDebug::TraceInformation(msg.Site.ThreadId, msg.Category->Name.data(), date.Value(), msg.Site.SourceFile, msg.Site.SourceLine, msg.Text());
            break;

        case ELoggerVerbosity::Warning:
            FPlatformDebug::TraceWarning(msg.Site.ThreadId, msg.Category->Name.data(), date.Value(), msg.Site.SourceFile, msg.Site.SourceLine, msg.Text());
            break;
        case ELoggerVerbosity::Error:
            FPlatformDebug::TraceError(msg.Site.ThreadId, msg.Category->Name.data(), date.Value(), msg.Site.SourceFile, msg.Site.SourceLine, msg.Text());
            break;
        case ELoggerVerbosity::Fatal:
            FPlatformDebug::TraceFatal(msg.Site.ThreadId, msg.Category->Name.data(), date.Value(), msg.Site.SourceFile, msg.Site.SourceLine, msg.Text());
            break;

        default:
            AssertNotImplemented();
        }
    }

    virtual void Flush(bool) override final {} // always synced
};
#endif //!USE_PPE_PLATFORM_DEBUG
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLogger::RegisterStdoutLogger() {
    if (FPlatformConsole::HasConsole)
        RegisterLogger(TRACKING_NEW(Logger, FConsoleWriterLogger_), true);
    else
        RegisterLogger(TRACKING_NEW(Logger, FFileHandleLogger_){FPlatformLowLevelIO::Stdout, false}, true);
}
//----------------------------------------------------------------------------
void FLogger::RegisterOutputDebugLogger() {
#if USE_PPE_PLATFORM_DEBUG
    RegisterLogger(TRACKING_NEW(Logger, FOutputDebugLogger_), true);
#endif
}
//----------------------------------------------------------------------------
void FLogger::RegisterAppendFileLogger(FConstWChar filename) {
    FPlatformLowLevelIO::FHandle hFile = FPlatformLowLevelIO::Open(filename,
            EOpenPolicy::Writable,
            EAccessPolicy::Create|EAccessPolicy::Append|EAccessPolicy::Binary|EAccessPolicy::ShareRead);
    AssertRelease(FPlatformLowLevelIO::InvalidHandle != hFile);
    RegisterLogger(TRACKING_NEW(Logger, FFileHandleLogger_){hFile}, true);
}
//----------------------------------------------------------------------------
void FLogger::RegisterAppendJsonLogger(FConstWChar filename) {
    FPlatformLowLevelIO::FHandle hFile = FPlatformLowLevelIO::Open(filename,
            EOpenPolicy::Writable,
            EAccessPolicy::Create|EAccessPolicy::Append|EAccessPolicy::Binary|EAccessPolicy::ShareRead);
    AssertRelease(FPlatformLowLevelIO::InvalidHandle != hFile);
    RegisterLogger(TRACKING_NEW(Logger, FJsonHandleLogger_){hFile}, true);
}
//----------------------------------------------------------------------------
void FLogger::RegisterRollFileLogger(FConstWChar filename) {
    Verify(FPlatformFile::RollFile(filename));
    RegisterAppendFileLogger(filename);
}
//----------------------------------------------------------------------------
void FLogger::RegisterRollJsonLogger(FConstWChar filename) {
    Verify(FPlatformFile::RollFile(filename));
    RegisterAppendJsonLogger(filename);
}
//----------------------------------------------------------------------------
void FLogger::RegisterSystemTraceLogger() {
#if USE_PPE_PLATFORM_DEBUG
    RegisterLogger(TRACKING_NEW(Logger, FSystemTraceLogger_), true);
#else
    return PLogger();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
TBasicConstChar<_Char> ELoggerVerbosity_Text_(ELoggerVerbosity level) {
    switch (level) {
    case FLogger::EVerbosity::None:      return STRING_LITERAL(_Char, "None").data();
    case FLogger::EVerbosity::Debug:     return STRING_LITERAL(_Char, "Debug").data();
    case FLogger::EVerbosity::Verbose:   return STRING_LITERAL(_Char, "Verbose").data();
    case FLogger::EVerbosity::Info:      return STRING_LITERAL(_Char, "Info").data();
    case FLogger::EVerbosity::Profiling: return STRING_LITERAL(_Char, "Profiling").data();
    case FLogger::EVerbosity::Emphasis:  return STRING_LITERAL(_Char, "Emphasis").data();
    case FLogger::EVerbosity::Warning:   return STRING_LITERAL(_Char, "Warning").data();
    case FLogger::EVerbosity::Error:     return STRING_LITERAL(_Char, "Error").data();
    case FLogger::EVerbosity::Fatal:     return STRING_LITERAL(_Char, "Fatal").data();
    default: break;
    }
    AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& ELoggerVerbosity_Oss_(TBasicTextWriter<_Char>& oss, ELoggerVerbosity level) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, '|'));
    if (level & FLogger::EVerbosity::Debug)     oss << sep << STRING_LITERAL(_Char, "Debug");
    if (level & FLogger::EVerbosity::Verbose)   oss << sep << STRING_LITERAL(_Char, "Verbose");
    if (level & FLogger::EVerbosity::Info)      oss << sep << STRING_LITERAL(_Char, "Info");
    if (level & FLogger::EVerbosity::Profiling) oss << sep << STRING_LITERAL(_Char, "Profiling");
    if (level & FLogger::EVerbosity::Emphasis)  oss << sep << STRING_LITERAL(_Char, "Emphasis");
    if (level & FLogger::EVerbosity::Warning)   oss << sep << STRING_LITERAL(_Char, "Warning");
    if (level & FLogger::EVerbosity::Error)     oss << sep << STRING_LITERAL(_Char, "Error");
    if (level & FLogger::EVerbosity::Fatal)     oss << sep << STRING_LITERAL(_Char, "Fatal");
    return oss;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
FConstChar ToString(FLogger::EVerbosity level) NOEXCEPT {
    return ELoggerVerbosity_Text_<char>(level);
}
//----------------------------------------------------------------------------
FConstWChar ToWString(FLogger::EVerbosity level) NOEXCEPT {
    return ELoggerVerbosity_Text_<wchar_t>(level);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, FLogger::EVerbosity level) {
    return ELoggerVerbosity_Oss_(oss, level);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, FLogger::EVerbosity level) {
    return ELoggerVerbosity_Oss_(oss, level);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_POP()
} //!namespace PPE

#endif //!USE_PPE_LOGGER
