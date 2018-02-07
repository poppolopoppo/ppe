#include "stdafx.h"

#include "Logger.h"

#ifdef USE_DEBUG_LOGGER

#   include "Container/Vector.h"
#   include "Diagnostic/CurrentProcess.h"
#   include "IO/BufferedStream.h"
#   include "IO/FileSystem.h"
#   include "IO/FileStream.h"
#   include "IO/FormatHelpers.h"
#   include "IO/StreamProvider.h"
#   include "IO/String.h"
#   include "IO/StringView.h"
#   include "IO/TextWriter.h"
#   include "IO/VirtualFileSystem.h"
#   include "Memory/MemoryView.h"
#   include "Meta/Optional.h"
#   include "Meta/ThreadResource.h"
#   include "Misc/TargetPlatform.h"
#   include "Time/DateTime.h"
#   include "Thread/Task/TaskHelpers.h"
#   include "Thread/Fiber.h"
#   include "Thread/ThreadContext.h"
#   include "Thread/ThreadPool.h"

#   include <atomic>
#   include <mutex>
#   include <iostream>

#   define CORE_DUMP_THREAD_ID              1
#   define CORE_DUMP_THREAD_NAME            0
#   define CORE_DUMP_SITE_ON_LOG            0
#   define CORE_DUMP_CALLSTACK_ON_WARNING   0
#   define CORE_DUMP_CALLSTACK_ON_ERROR     0

#   if (CORE_DUMP_CALLSTACK_ON_ERROR || CORE_DUMP_CALLSTACK_ON_WARNING)
#       include "Diagnostic/Callstack.h"
#       include "Diagnostic/DecodedCallstack.h"
#   endif

#   ifdef PLATFORM_WINDOWS
#       include "Misc/Platform_Windows.h"
#       include <wincon.h>
#   endif

namespace Core {
LOG_CATEGORY(CORE_API, LogDefault);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Used before & after main when no debugger is attached, ignores everything
class FNullLogger_ : public ILogger {
public: // ILogger
    virtual void Log(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&) override final {}
    virtual void Log(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&, size_t, size_t) override final {}
    virtual void Log(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&, const FWFormatArgList&) override final {}

    virtual void Flush(bool) override final {}
    virtual void Close() override final {}
};
//----------------------------------------------------------------------------
static void LogHeader_(FWTextWriter& oss, const ILogger::FCategory& category, ILogger::EVerbosity level, ILogger::FSiteInfo site) {
#if CORE_DUMP_THREAD_ID
#   if CORE_DUMP_THREAD_NAME
    Format(oss, L"[{0}][{1:20}][{3:-8}][{2:-15}] ", site.Timestamp.ToDateTimeUTC(), FThreadContext::GetThreadName(site.ThreadId), category.Name, level);
#   else // only thread hash :
    Format(oss, L"[{0}][{1:#5}][{3:-8}][{2:-15}] ", site.Timestamp.ToDateTimeUTC(), FThreadContext::GetThreadHash(site.ThreadId), category.Name, level);
#   endif
#else
    Format(oss, L"[{0}][{2:-8}][{1:-15}] ", site.Timestamp.ToDateTimeUTC(), category.Name, level); 
#endif
}
//----------------------------------------------------------------------------
static void LogFooter_(FWTextWriter& oss, const ILogger::FCategory& category, ILogger::EVerbosity level, ILogger::FSiteInfo site) {
#if CORE_DUMP_SITE_ON_LOG
    Format(oss, L"\n\tat {0}:{1}\n", site.Filename, site.Line);
#else
    oss << Eol;
#endif
}
//----------------------------------------------------------------------------
// Used before & after main when debugger attached, no dependencies on allocators, always immediate
class FPreMainOutputDebugLogger_ : public ILogger {
public: // ILogger
    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) override final {
        Assert(category.Verbosity & level);

        wchar_t tmp[8192];
        FWFixedSizeTextWriter oss(tmp);

        LogHeader_(oss, category, level, site);
        oss.Write(text);
        LogFooter_(oss, category, level, site);

        oss << Eos;

        FPlatformMisc::OutputDebug(tmp);
    }

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& buffer, size_t first, size_t last) override final {
        Assert(category.Verbosity & level);
        Assert(first < buffer.size());
        Assert(last <= buffer.size());

        wchar_t tmp[8192];
        FWFixedSizeTextWriter oss(tmp);

        LogHeader_(oss, category, level, site);
        if (first < last) {
            oss.Write(buffer.SubRange(first, last - first));
        }
        else {
            oss.Write(buffer.CutStartingAt(first));
            oss.Write(buffer.CutBefore(last));
        }
        LogFooter_(oss, category, level, site);

        oss << Eos;

        FPlatformMisc::OutputDebug(tmp);
    }

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) override final {
        Assert(category.Verbosity & level);

        wchar_t tmp[8192];
        FWFixedSizeTextWriter oss(tmp);

        LogHeader_(oss, category, level, site);
        FormatArgs(oss, format, args);
        LogFooter_(oss, category, level, site);

        oss << Eos;

        FPlatformMisc::OutputDebug(tmp);
    }

    virtual void Flush(bool) override final {}
    virtual void Close() override {}
};
//----------------------------------------------------------------------------
class FBucketedWriter_ {
public:
    FBucketedWriter_()
        : _buffer((u8*)Core::malloc(GBufferSize)) 
        , _bucketMask(GBucketMask) {
        Assert(_buffer);
#ifdef USE_MEMORY_DOMAINS
        MEMORY_DOMAIN_TRACKING_DATA(Diagnostic).Allocate(1, GBufferSize);
#endif
    }

    ~FBucketedWriter_() {
        Assert(_buffer);
        Assert(GBucketMask == _bucketMask);
#ifdef USE_MEMORY_DOMAINS
        MEMORY_DOMAIN_TRACKING_DATA(Diagnostic).Allocate(1, GBufferSize);
#endif
        Core::free(_buffer);
    }

    class FBucket {
    public:
        FBucket() 
            : _bucket(INDEX_NONE)
            , _writerCount(0) {}

        ~FBucket() {
            Assert(INDEX_NONE == _bucket);
            Assert(0 == _writerCount);
        }

        FBucket(const FBucket&) = delete;
        FBucket& operator =(const FBucket&) = delete;

        FWFixedSizeTextWriter& Open() {
            if (_writerCount) {
                Assert(INDEX_NONE != _bucket);
            }
            else {
                Assert(INDEX_NONE == _bucket);
                auto& instance = Instance_();
                _bucket = instance.AllocateBucket_();
                _ossIFP.emplace(instance.MakeView_(_bucket));
            }
            ++_writerCount;
            return (*_ossIFP);
        }

        void Close() {
            Assert(INDEX_NONE != _bucket);
            Assert(0 < _writerCount);
            if (--_writerCount == 0) {
                Instance_().ReleaseBucket_(_bucket);
                _bucket = INDEX_NONE;
                _ossIFP.reset();
            }
        }

    protected:
        size_t _bucket;
        int _writerCount;
        Meta::TOptional<FWFixedSizeTextWriter> _ossIFP;
    };

    class FScope : FBucket {
    public:
        FScope() { Open(); }
        ~FScope() { Close(); }
        FWFixedSizeTextWriter& Oss() { return (*_ossIFP); }
    };

private:
    STATIC_CONST_INTEGRAL(size_t, GNumBuckets, 4); // independent buckets
    STATIC_CONST_INTEGRAL(size_t, GBucketMask, GNumBuckets - 1);
    STATIC_CONST_INTEGRAL(size_t, GBucketSize, ALLOCATION_GRANULARITY); // 64k buckets
    STATIC_CONST_INTEGRAL(size_t, GBufferSize, GNumBuckets * GBucketSize); // <=> 256k
    STATIC_ASSERT(Meta::IsPow2(GNumBuckets));

    u8* const _buffer;
    std::atomic<size_t> _bucketMask;

    size_t AllocateBucket_() {
        for (;;) {
            size_t oldMask = _bucketMask;
            size_t bucket = Meta::tzcnt(oldMask);
            Assert(bucket < GNumBuckets);
            size_t newMask = (oldMask & ~(size_t(1) << bucket));
            AssertRelease(newMask != oldMask);
            if (_bucketMask.compare_exchange_strong(oldMask, newMask))
                return bucket;
        }
    }

    void ReleaseBucket_(size_t bucket) {
        Assert(bucket < GNumBuckets);
        for (;;) {
            size_t oldMask = _bucketMask;
            size_t newMask = (oldMask | (size_t(1) << bucket));
            AssertRelease(newMask != oldMask);
            if (_bucketMask.compare_exchange_strong(oldMask, newMask))
                break;
        }
    }

    TMemoryView<wchar_t> MakeView_(size_t bucket) const {
        return TMemoryView<u8>(_buffer + bucket * GBucketSize, GBucketSize).Cast<wchar_t>();
    }

    static FBucketedWriter_& Instance_() {
        ONE_TIME_DEFAULT_INITIALIZE(FBucketedWriter_, GInstance);
        return GInstance;
    }
};
//----------------------------------------------------------------------------
// Allows being called recursively
class FAbstractReentrantLogger_ : public ILogger {
public:
    FAbstractReentrantLogger_() 
        : _lockDepth(0)
        , _closing(false)
    {}

    virtual ~FAbstractReentrantLogger_() {
        Assert(0 == _lockDepth);
        Assert(_closing);
        Assert(_deferredCalls.empty());
    }

public: // ILogger
    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) override final {
        const FScopeLock_ scopeLock(*this);

        if (Unlikely(scopeLock.Closing())) {
            return; // discard messages after close
        }
        else if (Unlikely(scopeLock.Recursive())) {
            RecursiveLog_(category, level, site, text);
        }
        else {
            SafeLog(category, level, site, text);
            FlushDeferredCallsIFN_();
        }
    }

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& buffer, size_t first, size_t last) override final {
        const FScopeLock_ scopeLock(*this);

        if (Unlikely(scopeLock.Closing())) {
            return; // discard messages after close
        }
        else if (Unlikely(scopeLock.Recursive())) {
            AssertNotReached(); // logger setup should avoid that
        }
        else {
            SafeLog(category, level, site, buffer, first, last);
            FlushDeferredCallsIFN_();
        }
    }

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) override final {
        const FScopeLock_ scopeLock(*this);

        if (Unlikely(scopeLock.Closing())) {
            return; // discard messages after close
        }
        else if (Unlikely(scopeLock.Recursive())) {
            RecursiveLog_(category, level, site, format, args);
        }
        else {
            SafeLog(category, level, site, format, args);
            FlushDeferredCallsIFN_();
        }
    }

    virtual void Flush(bool synchronous) override {
        const FScopeLock_ scopeLock(*this);

        if (Unlikely(scopeLock.Closing())) {
            return; // discard flush after close
        }
        else if (Unlikely(scopeLock.Recursive())) {
            _deferredCalls.push_back([synchronous](FAbstractReentrantLogger_* logger) {
                logger->SafeFlush(synchronous);
            });
        }
        else {
            SafeFlush(synchronous);
            FlushDeferredCallsIFN_();
        }
    }

    virtual void Close() override final {
        const FScopeLock_ scopeLock(*this);

        if (scopeLock.Recursive()) {
            AssertNotReached();
        }
        else {
            Assert(not scopeLock.Closing());
            _closing = true;
            Assert(scopeLock.Closing());

            SafeFlush(true);
            FlushDeferredCallsIFN_();

            Assert(_deferredCalls.empty());

            SafeClose();

            Assert(_deferredCalls.empty());
        }
    }

protected:
    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) = 0;
    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& buffer, size_t first, size_t last) = 0;
    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) = 0;
    virtual void SafeFlush(bool synchronous) = 0;
    virtual void SafeClose() = 0;

    std::recursive_mutex _barrier;

private:
    int _lockDepth;
    bool _closing;

    FBucketedWriter_::FBucket _bucketWriter;

    struct FScopeLock_ {
        FAbstractReentrantLogger_& Owner;
        const Meta::FRecursiveLockGuard LockGuard;

        FScopeLock_(FAbstractReentrantLogger_& owner)
            : Owner(owner)
            , LockGuard(Owner._barrier) {
            ++Owner._lockDepth;
        }

        ~FScopeLock_() {
            --Owner._lockDepth;
        }

        bool Closing() const {
            Assert(Owner._lockDepth);
            return (Owner._closing);
        }

        bool Recursive() const {
            Assert(Owner._lockDepth);
            return (1 < Owner._lockDepth);
        }
    };

    using FDeferredCall_ = Meta::TFunction<void(FAbstractReentrantLogger_*)>;
    VECTORINSITU(Diagnostic, FDeferredCall_, 8) _deferredCalls;

    NO_INLINE void RecursiveLog_(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) {
        FWFixedSizeTextWriter& oss(_bucketWriter.Open());

        const size_t offText = oss.Tell();
        oss.Write(text);
        const FWStringView deferredText = oss.WrittenSince(offText);

        DeferLog_(oss, category, level, site, deferredText);
    }

    NO_INLINE void RecursiveLog_(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) {
        FWFixedSizeTextWriter& oss(_bucketWriter.Open());

        const size_t offText = oss.Tell();
        FormatArgs(oss, format, args);
        const FWStringView deferredText = oss.WrittenSince(offText);

        DeferLog_(oss, category, level, site, deferredText);
    }

    struct FDeferredLogEntry_ {
        const FLogger::FCategory* Category;
        FLogger::EVerbosity Level;
        FLogger::FSiteInfo Site;
        const wchar_t* Text;
        size_t Length;

        void Send(FAbstractReentrantLogger_* logger) const {
            logger->SafeLog(*Category, Level, Site, FWStringView(Text, Length));
            logger->_bucketWriter.Close();
        }
    };

    void DeferLog_(FWFixedSizeTextWriter& oss, const FCategory& category, EVerbosity level, const FSiteInfo& site, const FWStringView& deferredText) {
        // hack to store log entry infos inside text buffer and avoid another allocation
        const std::streamoff offEntry = ROUND_TO_NEXT_16(oss.Stream()->TellO()); // keep entry aligned
        oss.Stream()->WritePODAligned(FDeferredLogEntry_{ &category, level, site, deferredText.data(), deferredText.size() }, 16);

        // will be freed when closing the bucket
        const FDeferredLogEntry_* deferredEntry = reinterpret_cast<const FDeferredLogEntry_*>(
            oss.Stream()->WrittenSince(offEntry).data());

        _deferredCalls.emplace_back(deferredEntry, &FDeferredLogEntry_::Send);
    }

    void FlushDeferredCallsIFN_() {
        Assert(_lockDepth);
        if (_deferredCalls.empty())
            return;

        // can be muted by deferred calls, hence the slow/safe loop :
        size_t call = 0;
        do {
            _deferredCalls[call++](this);
        } while (call < _deferredCalls.size());

        _deferredCalls.clear();
    }
};
//----------------------------------------------------------------------------
// Composite logger managing all logger registrations
class FLoggerManager_ : public FAbstractReentrantLogger_ {
public:
    FLoggerManager_() {}
    virtual ~FLoggerManager_() {}

    void Register(ILogger* logger) {
        Assert(logger);
        const Meta::FRecursiveLockGuard scopeLock(_barrier);
        Add_AssertUnique(_logs, logger);
    }

    void Unregister(ILogger* logger) {
        Assert(logger);
        const Meta::FRecursiveLockGuard scopeLock(_barrier);
        Remove_AssertExists(_logs, logger);
        logger->Flush(true);
    }

protected: // FAbstractReentrantLogger_
    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) override final {
        for (ILogger* logger : _logs)
            logger->Log(category, level, site, text);
    }

    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& buffer, size_t first, size_t last) override final {
        for (ILogger* logger : _logs)
            logger->Log(category, level, site, buffer, first, last);
    }

    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) override final {
        for (ILogger* logger : _logs)
            logger->Log(category, level, site, format, args);
    }

    virtual void SafeFlush(bool synchronous) override final {
        for (ILogger* logger : _logs)
            logger->Flush(synchronous);
    }

    virtual void SafeClose() override final {
        for (ILogger* logger : _logs)
            logger->Close();
    }

private:
    VECTORINSITU(Diagnostic, ILogger*, 4) _logs;
};
//----------------------------------------------------------------------------
static ILogger* GLogger_ = nullptr;
static bool GLoggerAvailable_ = false;
static bool GLoggerImmediate_ = true; // set to false in FLogger::Start() and reverted to false in FLogger::Shutdown()
static POD_STORAGE(FLoggerManager_) GLoggerStorage_;
//----------------------------------------------------------------------------
template <typename T>
static void SetupInplaceLogger_() {
    if (GLogger_) {
        GLogger_->Close();
        GLogger_->~ILogger();
    }

    STATIC_ASSERT(sizeof(T) <= sizeof(GLoggerStorage_));
    GLogger_ = static_cast<ILogger*>(new ((void*)&GLoggerStorage_) T);
}
//----------------------------------------------------------------------------
static NO_INLINE void SetupPreMainLogger_() {
    if (FPlatformMisc::IsDebuggerAttached())
        SetupInplaceLogger_<FPreMainOutputDebugLogger_>();
    else
        SetupInplaceLogger_<FNullLogger_>();
}
//----------------------------------------------------------------------------
static ILogger& Logger_() 
{
    // fall back for pre-logger start
    if (Unlikely(nullptr == GLogger_))
        SetupPreMainLogger_();

    return (*GLogger_);
}
//----------------------------------------------------------------------------
static FLoggerManager_& LoggerManager_() {
    Assert(GLoggerAvailable_);
    return reinterpret_cast<FLoggerManager_&>(GLoggerStorage_);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static void HandleFatalLogIFN_(FLogger::EVerbosity level) {
    if (FLogger::EVerbosity::Fatal == level) {
        Logger_().Flush(true);
        AssertNotReached();
    }
}
//----------------------------------------------------------------------------
void FLogger::Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) {
    Logger_().Log(category, level, site, text);
    HandleFatalLogIFN_(level);
}
//----------------------------------------------------------------------------
void FLogger::LogArgs(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) {
    Logger_().Log(category, level, site, format, args);
    HandleFatalLogIFN_(level);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FAsynchronousLogger_ : public FAbstractReentrantLogger_, private IBufferedStreamWriter {
public:
    FAsynchronousLogger_(ILogger* wrapped)
        : _wrapped(wrapped)
        , _ringBuffer((u8*)Core::malloc(GBufferSize), GBufferSize)
        , _offsetO(0)
        , _offsetI(0)
        , _sizeInBytes(0) {
        AssertRelease(_wrapped);
#ifdef USE_MEMORY_DOMAINS
        MEMORY_DOMAIN_TRACKING_DATA(Diagnostic).Allocate(1, GBufferSize);
#endif
    }

    ~FAsynchronousLogger_() {
        Assert(_offsetI == checked_cast<size_t>(_offsetO & GBufferMask)); // missing Flush() ?
#ifdef USE_MEMORY_DOMAINS
        MEMORY_DOMAIN_TRACKING_DATA(Diagnostic).Deallocate(1, GBufferSize);
#endif
        Core::free(_ringBuffer.data());
    }

    ILogger* Wrapped() const { return _wrapped; }
    void SetWrapped(ILogger* wrapped) { _wrapped = wrapped; }

    using FAbstractReentrantLogger_::Flush;

public: // ILogger
    virtual void Flush(bool synchronous) override final {
        synchronous &= (not FFiber::IsInFiber());
        AsyncFlush_(_wrapped, synchronous);
    }

protected: // FAbstractReentrantLogger_
    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) override final {
        const std::streamoff offset = _offsetO;
        static_cast<IBufferedStreamWriter*>(this)->WriteView(text);
        AsyncLog_(category, level, site, offset);
    }

    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& buffer, size_t first, size_t last) override final {
        AssertNotImplemented(); // shouldn't be called since only this logger uses this interface
    }

    virtual void SafeLog(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) override final {
        const std::streamoff offset = _offsetO;
        FWTextWriter oss(static_cast<IBufferedStreamWriter*>(this));
        FormatArgs(oss, format, args);
        AsyncLog_(category, level, site, offset);
    }

    virtual void SafeFlush(bool synchronous) override final {
        AssertNotImplemented(); // run without barrier to avoid dead lock
    }

    virtual void SafeClose() override final {
        AssertNotImplemented(); // shouldn't be closed
    }

private: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const override final { UNUSED(origin); return false; }

    virtual std::streamoff TellO() const override final { return _offsetO; }
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final { 
        UNUSED(offset);
        UNUSED(origin);
        AssertNotImplemented();
    }

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final {
        AssertRelease(checked_cast<size_t>(sizeInBytes) < _ringBuffer.SizeInBytes());
        if (0 == sizeInBytes)
            return true;
        
        const TMemoryView<const u8> toWrite((const u8*)storage, sizeInBytes);
        const size_t off0 = checked_cast<size_t>(_offsetO & GBufferMask);
        const size_t off1 = checked_cast<size_t>((_offsetO + sizeInBytes) & GBufferMask);

        while (_sizeInBytes + sizeInBytes > GBufferSize) {
            _barrier.unlock(); // release the lock while flushing
            Flush(true); // flush synchronous which will wait for all delayed logs
            _barrier.lock();
        }

        if (off0 < off1) {
            toWrite.CopyTo(_ringBuffer.SubRange(off0, off1 - off0));
        }
        else {
            const size_t split = (GBufferSize - off0);
            Assert(toWrite.size() - split == off1);

            std::copy(toWrite.begin(), toWrite.begin() + split, _ringBuffer.begin() + off0);
            std::copy(toWrite.begin() + split, toWrite.end(), _ringBuffer.begin());
        }

        _offsetO += sizeInBytes;
        _sizeInBytes += checked_cast<size_t>(sizeInBytes);
        Assert(_sizeInBytes <= GBufferSize);

        return true;
    }

    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final {
        return (FAsynchronousLogger_::Write(storage, eltsize * count) ? count : 0);
    }

private: // IBufferedStreamWriter
    virtual void Flush() override final {}

private:
    // 1024k, must be large enough to let the asynchronous tasks complete
    STATIC_CONST_INTEGRAL(size_t, GBufferSize, ALLOCATION_GRANULARITY*16); 
    STATIC_CONST_INTEGRAL(size_t, GBufferMask, GBufferSize - 1);
    STATIC_ASSERT(Meta::IsPow2(GBufferSize));

    ILogger* _wrapped;
    TMemoryView<u8> _ringBuffer;
    std::streamoff _offsetO;
    std::atomic<size_t> _offsetI;
    std::atomic<size_t> _sizeInBytes;

    static FTaskManager& TaskManager_() {
        FTaskManager& manager = FLowestPriorityThreadPool::Instance();
        Assert(manager.WorkerCount() == 1); // wrapped loggers ain't necessarily thread-safe
        return manager;
    }

    static void AsyncFlush_(ILogger* wrapped, bool synchronous) { 
        FTaskFunc flushTask = [wrapped](ITaskContext& ctx) {
            wrapped->Flush(true); // flush synchronously in asynchronous task
            Core::malloc_release_pending_blocks(); // not a bad time for releasing some pending blocks
        };
        const ETaskPriority flushPriority = ETaskPriority::Low/* low priority to be sure to be executed last */;
        if (synchronous)
            TaskManager_().RunAndWaitFor(std::move(flushTask), flushPriority);
        else
            TaskManager_().Run(std::move(flushTask), flushPriority);
    }

    class FAsyncLogEntry_ {
    public:
        FAsyncLogEntry_(
            FAsynchronousLogger_ * logger,
            const FCategory& category,
            EVerbosity level,
            FSiteInfo site,
            size_t off0,
            size_t off1,
            size_t off2)
        :   _logger(logger)
        ,   _category(&category)
        ,   _level(level)
        ,   _site(site)
        ,   _off0(off0)
        ,   _off1(off1)
        ,   _off2(off2) {
            Assert_NoAssume(CheckCanary_());
        }

        void Run(ITaskContext&) {
            Assert_NoAssume(CheckCanary_());

            _logger->_wrapped->Log(*_category, _level, _site, 
                _logger->_ringBuffer.Cast<const wchar_t>(), 
                _off0 / sizeof(wchar_t), 
                _off1 / sizeof(wchar_t) );

            Assert_NoAssume(CheckCanary_());

            const size_t entrySize = (_off0 < _off2 ? _off2 - _off0 : GBufferSize - _off0 + _off2);
            const size_t oldSize = _logger->_sizeInBytes.fetch_sub(entrySize);
            Assert(oldSize >= entrySize);
            Assert(_logger->_sizeInBytes <= GBufferSize);

            Assert_NoAssume(CheckCanary_());

            // after this call this object is no more guaranteed to be valid !
            if (not _logger->_offsetI.compare_exchange_strong(_off0, _off2))
                AssertNotReached(); // should be run in order by task manager !
        }

    private:
#ifdef WITH_CORE_ASSERT
        STATIC_CONST_INTEGRAL(size_t, Canary, CODE3264(0xdeadbeeful, 0xdeadbeefdeadbeefull));
        size_t _canary0 = Canary;
#endif
        FAsynchronousLogger_ * _logger;
        const FCategory* _category;
        EVerbosity _level;
        FSiteInfo _site;
        size_t _off0;
        size_t _off1;
        size_t _off2;
#ifdef WITH_CORE_ASSERT
        size_t _canary1 = Canary;
        bool CheckCanary_() const {
            return (Canary == _canary0 && Canary == _canary1);
        }
#endif
    };

    void AsyncLog_(const FCategory& category, EVerbosity level, const FSiteInfo& site, std::streamoff beg) {
        const size_t off0 = checked_cast<size_t>(beg & GBufferMask);
        const size_t off1 = checked_cast<size_t>(_offsetO & GBufferMask);

        // hack to store async log entry inside text buffer and avoid another allocation
        size_t offEntry = (ROUND_TO_NEXT_16(off1) & GBufferMask);
        size_t sizeFooter = (offEntry - off1 + sizeof(FAsyncLogEntry_));
        if (((off1 + sizeFooter) & GBufferMask) < offEntry) { // handles wrapping with padding : 
            const size_t padding = (GBufferSize - offEntry);
            Assert(padding < sizeof(FAsyncLogEntry_));
            Assert(((offEntry + padding) & GBufferMask) == 0);

            offEntry = 0;
            sizeFooter += padding;
        }

        _offsetO += sizeFooter;
        _sizeInBytes += sizeFooter;
        Assert(_sizeInBytes <= GBufferSize);

        const size_t off2 = checked_cast<size_t>(_offsetO & GBufferMask);

        // will be freed on another thread when _offsetI will be incremented
        FAsyncLogEntry_* pEntry = new ((void*)&_ringBuffer[offEntry]) FAsyncLogEntry_(
            this, category, level, site, off0, off1, off2 );

        Assert((void*)(pEntry + 1) == (void*)(&_ringBuffer[off2]));

        TaskManager_().Run(FTaskFunc(pEntry, &FAsyncLogEntry_::Run), ETaskPriority::Normal);
    }
};
//----------------------------------------------------------------------------
static POD_STORAGE(FAsynchronousLogger_) GAsyncLoggerStorage_;
//----------------------------------------------------------------------------
static bool SetupAsyncrhonousLogger_(bool immediate) {
    Assert(GLogger_);

    if (immediate) {
        Assert((void*)GLogger_ == &GAsyncLoggerStorage_);

        ILogger* const wrapped = checked_cast<FAsynchronousLogger_*>(GLogger_)->Wrapped();
        FAsynchronousLogger_* const asynchronous = checked_cast<FAsynchronousLogger_*>(GLogger_);

        // flush a first time previous logs
        asynchronous->Flush(true);

        // fall back to low level logger before flushing a second time
        FPreMainOutputDebugLogger_ postAsync;
        GLogger_ = &postAsync;
        asynchronous->Flush(true);

        // finally restore wrapped logger bestore destroying asynchronous logger
        GLogger_ = wrapped;
        asynchronous->~FAsynchronousLogger_();

        return true;
    }
    else {
        Assert((void*)GLogger_ != &GAsyncLoggerStorage_);
        
        // flush before installing asynchronous logger
        GLogger_->Flush(true);

        GLogger_ = new ((void*)&GAsyncLoggerStorage_) FAsynchronousLogger_(GLogger_);

        return false;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLogger::Start() {
    Assert(not GLoggerAvailable_);
    Assert(GLoggerImmediate_);

    GLoggerAvailable_ = true;
    SetupInplaceLogger_<FLoggerManager_>();

    RegisterLogger(FCurrentProcess::Instance().StartedWithDebugger()
        ? FLogger::OutputDebug()
        : FLogger::Stdout() );

    RegisterLogger(FLogger::RollFile(L"Saved:/Log/Core.log"));

    SetImmediate(false);
}
//----------------------------------------------------------------------------
void FLogger::Shutdown() {
    Assert(GLoggerAvailable_);

    SetImmediate(true);
    GLoggerAvailable_ = false;

    // revert to low level logger while the program is exiting
    SetupPreMainLogger_();
}
//----------------------------------------------------------------------------
bool FLogger::SetImmediate(bool immediate) {
    Assert(GLoggerAvailable_);
#if USE_CORE_MEMORY_DEBUGGING
    immediate = true; // forcibly disable asynchronous logger to ease debugging
#endif

    return (immediate != GLoggerImmediate_)
        ? not (GLoggerImmediate_ = SetupAsyncrhonousLogger_(immediate))
        : immediate;
}
//----------------------------------------------------------------------------
void FLogger::Flush(bool synchronous/* = true */) {
    if (GLoggerAvailable_)
        GLogger_->Flush(synchronous);
}
//----------------------------------------------------------------------------
void FLogger::RegisterLogger(ILogger* logger) {
    Assert(GLoggerAvailable_);

    LoggerManager_().Register(logger);
}
//----------------------------------------------------------------------------
void FLogger::UnregisterLogger(ILogger* logger) {
    Assert(GLoggerAvailable_);

    LoggerManager_().Unregister(logger);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FAbstractSuicideLogger_ : public ILogger {
public:
    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) override final {
        FBucketedWriter_::FScope log;
        BeginWrite(log.Oss(), category, level, site);
        log.Oss() << text;
        EndWrite(log.Oss(), category, level, site);
    }

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& buffer, size_t first, size_t last) override final {
        FBucketedWriter_::FScope log;
        BeginWrite(log.Oss(), category, level, site);
        if (first < last) {
            log.Oss().Write(buffer.SubRange(first, last - first));
        }
        else {
            log.Oss().Write(buffer.CutStartingAt(first));
            log.Oss().Write(buffer.CutBefore(last));
        }
        EndWrite(log.Oss(), category, level, site);
    }

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) override final {
        FBucketedWriter_::FScope log;
        BeginWrite(log.Oss(), category, level, site);
        FormatArgs(log.Oss(), format, args);
        EndWrite(log.Oss(), category, level, site);
    }

    virtual void Close() override final {
        Flush(true);
        checked_delete(this);
    }

protected:
    virtual void BeginWrite(FWFixedSizeTextWriter& oss, const FCategory& category, EVerbosity level, FSiteInfo site) {
        LogHeader_(oss, category, level, site);
    }
    virtual void EndWrite(FWFixedSizeTextWriter& oss, const FCategory& category, EVerbosity level, FSiteInfo site) {
        LogFooter_(oss, category, level, site);
    }
};
//----------------------------------------------------------------------------
class FOutputDebugLogger_ : public FAbstractSuicideLogger_ {
public:
    typedef FAbstractSuicideLogger_ parent_type;

    virtual void Flush(bool) override final {}

    virtual void EndWrite(FWFixedSizeTextWriter& oss, const FCategory& category, EVerbosity level, FSiteInfo site) override final {
        parent_type::EndWrite(oss, category, level, site);
        oss << Eos;
        FPlatformMisc::OutputDebug(oss.Written().data());
    }
};
//----------------------------------------------------------------------------
class FStdoutLogger_ : public FAbstractSuicideLogger_ {
public:
    typedef FAbstractSuicideLogger_ parent_type;

    virtual void Flush(bool) override final {
        fflush(stdout);
    }

    virtual void EndWrite(FWFixedSizeTextWriter& oss, const FCategory& category, EVerbosity level, FSiteInfo site) override final {
        parent_type::EndWrite(oss, category, level, site);
        oss << Eos;
        fputws(oss.Written().data(), stdout);
    }
};
//----------------------------------------------------------------------------
class FFunctorLogger_ : public FAbstractSuicideLogger_ {
public:
    typedef FAbstractSuicideLogger_ parent_type;
    typedef Meta::TFunction<void(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&)> functor_type;

    FFunctorLogger_(functor_type&& func)
        : _func(std::move(func))
    {}

    virtual void Flush(bool) override final {}

protected:
    virtual void BeginWrite(FWFixedSizeTextWriter& oss, const FCategory& category, EVerbosity level, FSiteInfo site) override final {}
    virtual void EndWrite(FWFixedSizeTextWriter& oss, const FCategory& category, EVerbosity level, FSiteInfo site) override final {
        parent_type::EndWrite(oss, category, level, site);
        _func(category, level, site, oss.Written());
    }

private:
    functor_type _func;
};
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
class FConsoleWriterLogger_ : public FAbstractSuicideLogger_ {
public:
    typedef FAbstractSuicideLogger_ parent_type;

    FConsoleWriterLogger_()
        : _hConsole(nullptr)
        , _textAttribute(::WORD(-1)) 
        , _pNewStdout(nullptr)
        , _pNewStderr(nullptr)
        , _pNewStdin(nullptr) {
        if (::AllocConsole()) {
            Verify(_hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE));

            ::SetConsoleMode(_hConsole, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
            ::SetConsoleOutputCP(CP_UTF8);

            // Redirect CRT standard input, output and error handles to the console window.
            ::freopen_s(&_pNewStdout, "CONOUT$", "w", stdout);
            ::freopen_s(&_pNewStderr, "CONOUT$", "w", stderr);
            ::freopen_s(&_pNewStdin, "CONIN$", "r", stdin);

            // Clear the error state for all of the C++ standard streams. Attempting to accessing the streams before they refer
            // to a valid target causes the stream to enter an error state. Clearing the error state will fix this problem,
            // which seems to occur in newer version of Visual Studio even when the console has not been read from or written
            // to yet.
            std::cout.clear();
            std::cerr.clear();
            std::cin.clear();

            std::wcout.clear();
            std::wcerr.clear();
            std::wcin.clear();
        }
    }

    ~FConsoleWriterLogger_() {
        if (_hConsole) {
#if 0
            // Redirect back CRT standard input, output and error handles
            ::freopen_s(&_pNewStdout, "CONOUT$", "w", stdout);
            ::freopen_s(&_pNewStderr, "CONOUT$", "w", stderr);
            ::freopen_s(&_pNewStdin, "CONIN$", "r", stdin);
#endif

            ::FreeConsole();
        }
    }

    virtual void Flush(bool) override final {}

protected:
    virtual void EndWrite(FWFixedSizeTextWriter& oss, const FCategory& category, EVerbosity level, FSiteInfo site) override final {
        parent_type::EndWrite(oss, category, level, site);

        constexpr ::WORD fWhite = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        constexpr ::WORD fYellow = FOREGROUND_RED | FOREGROUND_GREEN;
        constexpr ::WORD fCyan = FOREGROUND_GREEN | FOREGROUND_BLUE;
        constexpr ::WORD bBlack = 0;

        ::WORD textAttribute;
        switch (level)
        {
        case EVerbosity::Info:
            textAttribute = (fWhite | bBlack);
            break;
        case EVerbosity::Emphasis:
            textAttribute = (FOREGROUND_GREEN | BACKGROUND_BLUE | FOREGROUND_INTENSITY);
            break;
        case EVerbosity::Warning:
            textAttribute = (fYellow | bBlack | FOREGROUND_INTENSITY);
            break;
        case EVerbosity::Error:
            textAttribute = (FOREGROUND_RED | bBlack);
            break;
        case EVerbosity::Debug:
            textAttribute = (fCyan | bBlack);
            break;
        case EVerbosity::Fatal:
            textAttribute = (fWhite | BACKGROUND_RED | BACKGROUND_INTENSITY);
            break;
        default:
            textAttribute = (fWhite | bBlack);
            break;
        }

        if (_textAttribute != textAttribute) {
            _textAttribute = textAttribute;
            Verify(::SetConsoleTextAttribute(_hConsole, textAttribute));
        }

        const FWStringView towrite = oss.Written();
        ::DWORD written = checked_cast<::DWORD>(towrite.size());
        Verify(::WriteConsoleW(_hConsole, towrite.data(), written, &written, nullptr));
    }

private:
    ::HANDLE _hConsole;
    ::WORD _textAttribute;
    ::FILE* _pNewStdout = nullptr;
    ::FILE* _pNewStderr = nullptr;
    ::FILE* _pNewStdin = nullptr;
};
#endif //!PLATFORM_WINDOWS
//----------------------------------------------------------------------------
class FStreamLogger_ : public ILogger {
public:
    FStreamLogger_(UStreamWriter&& ostream)
        : _ostream(std::move(ostream))
        , _buffered(_ostream.get())
    {}

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& text) override final {
        FWTextWriter oss(&_buffered);
        LogHeader_(oss, category, level, site);
        oss << text;
        LogFooter_(oss, category, level, site);
    }

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& buffer, size_t first, size_t last) override final {
        FWTextWriter oss(&_buffered);
        LogHeader_(oss, category, level, site);
        if (first < last) {
            oss.Write(buffer.SubRange(first, last - first));
        }
        else {
            oss.Write(buffer.CutStartingAt(first));
            oss.Write(buffer.CutBefore(last));
        }
        LogFooter_(oss, category, level, site);
    }

    virtual void Log(const FCategory& category, EVerbosity level, FSiteInfo site, const FWStringView& format, const FWFormatArgList& args) override final {
        FWTextWriter oss(&_buffered);
        LogHeader_(oss, category, level, site);
        FormatArgs(oss, format, args);
        LogFooter_(oss, category, level, site);
    }

    virtual void Flush(bool) override final {
        _buffered.Flush();
    }

    virtual void Close() override final {
        Flush(true);
        checked_delete(this);
    }

private:
    UStreamWriter _ostream;
    FBufferedStreamWriter _buffered;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ILogger* FLogger::Stdout() {
#ifdef PLATFORM_WINDOWS
    return new FConsoleWriterLogger_;
#else
    return new FStdoutLogger_;
#endif
}
//----------------------------------------------------------------------------
ILogger* FLogger::OutputDebug() {
    return new FOutputDebugLogger_;
}
//----------------------------------------------------------------------------
ILogger* FLogger::AppendFile(const wchar_t* filename) {
    const FFilename fname(MakeCStringView(filename));
    UStreamWriter ostream = VFS_OpenWritable(fname, EAccessPolicy::Create|EAccessPolicy::Append|EAccessPolicy::Binary);
    AssertRelease(ostream);
    return new FStreamLogger_(std::move(ostream));
}
//----------------------------------------------------------------------------
ILogger* FLogger::RollFile(const wchar_t* filename) {
    const FFilename fname(MakeCStringView(filename));
    UStreamWriter ostream = VFS_RollFile(fname, EAccessPolicy::Create|EAccessPolicy::Truncate|EAccessPolicy::Binary);
    AssertRelease(ostream);
    return new FStreamLogger_(std::move(ostream));
}
//----------------------------------------------------------------------------
ILogger* FLogger::Functor(Meta::TFunction<void(const FCategory&, EVerbosity, FSiteInfo, const FWStringView&)>&& write) {
    Assert(write);
    return new FFunctorLogger_(std::move(write));
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
} //!namespace Core

#endif //!USE_DEBUG_LOGGER
