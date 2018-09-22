#pragma once

#include "Core.h"

#ifndef USE_PPE_MALLOC_LEAKDETECTOR
#   ifdef ARCH_X64
#       define USE_PPE_MALLOC_LEAKDETECTOR (0) // turn to 1 to detect leaks in the app, best results with USE_PPE_MEMORY_DEBUGGING=1 %_NOCOMMIT%
#   else
#       define USE_PPE_MALLOC_LEAKDETECTOR (0) // not available on 32 bits because of FBlockHeader+FCompressedRadixTrie
#   endif
#endif

#if USE_PPE_MALLOC_LEAKDETECTOR

#include "Container/CompressedRadixTrie.h"
#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "Diagnostic/Callstack.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/DecodedCallstack.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformLowLevelIO.h"
#include "IO/FileStream.h"
#include "IO/FormatHelpers.h"
#include "IO/StreamProvider.h"
#include "Memory/HashFunctions.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/MemoryView.h"
#include "Memory/VirtualMemory.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/ThreadContext.h"

namespace PPE {
LOG_CATEGORY_VERBOSITY(, Leaks, NoDebug)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLeakDetector {
public:
    struct FBlockHeader {
        // Packing will work for little-endian, beware for big-endian !
        u32 const Reserved  : 1;    // Must always be == 0 !
        u32 Enabled         : 1;    // 2
        u32 SizeInBytes     : 30;   // 32
        u32 CallstackUID;

        FBlockHeader() : Reserved(0) {}

        u64 Pack() const { return (*reinterpret_cast<const u64*>(this)); }
        static FBlockHeader Unpack(u64 v) { return (*reinterpret_cast<FBlockHeader*>(&v)); }
    };
    STATIC_ASSERT(sizeof(FBlockHeader) == sizeof(u64));

    struct FCallstackHeader {
        u64 Fingerprint;
        u64 StreamOffset    : 62;
        u64 KnownTrimmer    : 1;
        u64 KnownDeleter    : 1;

        bool IsNonDeleter() const { return (0 == (KnownDeleter | KnownTrimmer)); }
    };
    STATIC_ASSERT(sizeof(FCallstackHeader) == sizeof(u64) * 2);

    struct FCallstackData {
        STATIC_CONST_INTEGRAL(size_t, FramesToSkip, 3);
        STATIC_CONST_INTEGRAL(size_t, MaxDepth, 32);

        void* Frames[MaxDepth] = { 0 };

        void CaptureBacktrace() {
            FCallstack::Capture(PPE::MakeView(Frames), nullptr, FramesToSkip, MaxDepth);
        }

        void Decode(FDecodedCallstack* decoded) const {
            size_t depth = 0;
            for (void* frame : Frames) {
                if (nullptr == frame) break;
                depth++;
            }

            const auto frames = MakeConstView(Frames).CutBefore(depth);
            Verify(FCallstack::Decode(decoded, 0, frames));
        }

        u64 MakeFingerprint() const { return Fingerprint64(MakeConstView(Frames)); }
    };

    struct FCallstackBlocks {
        u32 CallstackUID;
        u32 NumAllocs;
        u32 MinSizeInBytes;
        u32 MaxSizeInBytes;
        u32 TotalSizeInBytes;

        FCallstackBlocks() : FCallstackBlocks(UINT32_MAX) {}
        explicit FCallstackBlocks(u32 callstackUID)
            : CallstackUID(callstackUID)
            , NumAllocs(0)
            , MinSizeInBytes(UINT32_MAX)
            , MaxSizeInBytes(0)
            , TotalSizeInBytes(0)
        {}

        void Add(const FBlockHeader& alloc) {
            Assert(alloc.CallstackUID == CallstackUID);
            NumAllocs++;
            MinSizeInBytes = Min(MinSizeInBytes, alloc.SizeInBytes);
            MaxSizeInBytes = Max(MaxSizeInBytes, alloc.SizeInBytes);
            TotalSizeInBytes += alloc.SizeInBytes;
        }
    };


    enum EReportMode {
        ReportOnlyLeaks,
        ReportOnlyNonDeleters,
        ReportAllBlocks,
    };

    struct FLeakReport {
        EReportMode Mode;
        u32 NumAllocs;
        u32 MinSizeInBytes;
        u32 MaxSizeInBytes;
        u32 TotalSizeInBytes;
        VECTOR(Internal, FCallstackBlocks) Callstacks;

        explicit FLeakReport(EReportMode mode)
            : Mode(mode)
            , NumAllocs(0)
            , MinSizeInBytes(UINT32_MAX)
            , MaxSizeInBytes(0)
            , TotalSizeInBytes(0)
        {}
    };

    static bool& WhiteListedTLS() {
        ONE_TIME_INITIALIZE_THREAD_LOCAL(bool, GWhiteListedTLS, false);
        return GWhiteListedTLS;
    }

    struct FWhiteListScope {
        bool WasWhiteListed;
        FWhiteListScope() : WasWhiteListed(WhiteListedTLS()) { WhiteListedTLS() = true; }
        ~FWhiteListScope() { WhiteListedTLS() = WasWhiteListed; }
    };

    static FLeakDetector& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FLeakDetector, GInstance);
        return GInstance;
    }

    bool IsEnabled() const { return _enabled; }

    void Start() {
        _callstacks.OpenStream(); // force create the file stream
        _enabled = true;
    }

    void Shutdown() {
        _enabled = false;
        ReportLeaks();
    }

    bool DontDisregard() const { return (_enabled && not WhiteListedTLS()); }

    void Allocate(void* ptr, size_t sizeInBytes) {
        const FRecursiveLockTLS reentrantLock;
        if (reentrantLock.WasLocked)
            return;

        Assert(ptr);
        Assert(sizeInBytes);

        FCallstackData callstackData;
        callstackData.CaptureBacktrace();

        FBlockHeader alloc;
        alloc.Enabled = DontDisregard();
        alloc.SizeInBytes = sizeInBytes;
        alloc.CallstackUID = _callstacks.FindOrAdd(callstackData);

        _blocks.Allocate(ptr, alloc);
    }

#if 0
    void Realloc(void* newPtr, size_t newSizeInBytes, void* oldPtr) {
        const FRecursiveLockTLS reentrantLock;
        if (reentrantLock.WasLocked)
            return;

        Assert(newPtr);
        Assert(oldPtr);
        Assert(newSizeInBytes);

        const FBlockHeader oldAlloc = _blocks.Release(oldPtr);

        if (oldAlloc.SizeInBytes > newSizeInBytes)
            _callstacks.Trim(oldAlloc.CallstackUID);

        FCallstackData callstackData;
        callstackData.CaptureBacktrace();

        FBlockHeader newAlloc;
        newAlloc.Enabled = DontDisregard();
        newAlloc.SizeInBytes = newSizeInBytes;
        newAlloc.CallstackUID = _callstacks.FindOrAdd(callstackData);

        _blocks.Allocate(newPtr, newAlloc);
    }
#endif

    void Release(void* ptr) {
        const FRecursiveLockTLS reentrantLock;
        if (reentrantLock.WasLocked)
            return;

        Assert(ptr);

        const FBlockHeader alloc = _blocks.Release(ptr);
        _callstacks.Delete(alloc.CallstackUID);
    }

    void FindLeaks(FLeakReport* report) {
        HASHMAP(Internal, u32, u32) callstackUIDtoIndex;
        callstackUIDtoIndex.reserve(_callstacks.NumCallstacks);
        report->Callstacks.reserve(_callstacks.NumCallstacks);

        const FRecursiveLockTLS reentrantLock; // make sure

        _blocks.Foreach([&, this, report](void* /*ptr*/, const FBlockHeader& alloc) {
            if (report->Mode == ReportAllBlocks ||
                (alloc.Enabled && (report->Mode != ReportOnlyNonDeleters ||
                    _callstacks.IsNonDeleter(alloc.CallstackUID))) ) {
                report->NumAllocs++;
                report->MinSizeInBytes = Min(report->MinSizeInBytes, alloc.SizeInBytes);
                report->MaxSizeInBytes = Max(report->MaxSizeInBytes, alloc.SizeInBytes);
                report->TotalSizeInBytes += alloc.SizeInBytes;

                const auto it = callstackUIDtoIndex.try_emplace(alloc.CallstackUID);
                if (it.second) {
                    it.first->second = checked_cast<u32>(report->Callstacks.size());
                    report->Callstacks.emplace_back(alloc.CallstackUID);
                }

                FCallstackBlocks& callstack = report->Callstacks[it.first->second];
                Assert(callstack.CallstackUID == alloc.CallstackUID);
                callstack.Add(alloc);
            }
        });

        std::sort(report->Callstacks.begin(), report->Callstacks.end(),
            [](const FCallstackBlocks& lhs, const FCallstackBlocks& rhs) {
                return (lhs.TotalSizeInBytes > rhs.TotalSizeInBytes); // sort from biggest leak to smallest
            });
    }

    void ReportLeaks(EReportMode mode = ReportOnlyLeaks) {
        _callstacks.Flush();

        FLeakReport report(mode);
        FindLeaks(&report);

        if (0 == report.NumAllocs) {
            Assert(report.Callstacks.empty());
            return;
        }

        LOG(Leaks, Error, L"Found {0} leaking blocks, total {1} [{2}, {3}] (mode = {4})",
            Fmt::CountOfElements(report.NumAllocs),
            Fmt::SizeInBytes(report.TotalSizeInBytes),
            Fmt::SizeInBytes(report.MinSizeInBytes),
            Fmt::SizeInBytes(report.MaxSizeInBytes),
            report.Mode == ReportAllBlocks
                ? "all blocks" : (report.Mode == ReportOnlyNonDeleters
                    ? "only non deleters" : "all leaks") );

        FCallstackHeader callstackHeader;
        FCallstackData callstackData;
        FDecodedCallstack decodedCallstack;
        for (const FCallstackBlocks& callstack : report.Callstacks) {
            _callstacks.Fetch(callstack.CallstackUID, &callstackHeader, &callstackData);

            callstackData.Decode(&decodedCallstack);

            LOG(Leaks, Error, L"{0} leaking blocks, total {1} [{2}, {3}], known trimmers = {4:a}, known deleters = {5:a}\n{6}",
                Fmt::CountOfElements(callstack.NumAllocs),
                Fmt::SizeInBytes(callstack.TotalSizeInBytes),
                Fmt::SizeInBytes(callstack.MinSizeInBytes),
                Fmt::SizeInBytes(callstack.MaxSizeInBytes),
                (callstackHeader.KnownTrimmer != 0),
                (callstackHeader.KnownDeleter != 0),
                decodedCallstack );
        }

        FLUSH_LOG();
    }

private:
    struct FRecursiveLockTLS {
        bool WasLocked;
        FRecursiveLockTLS() : WasLocked(Get()) { Get() = true; }
        ~FRecursiveLockTLS() { Get() = WasLocked; }
        static bool& Get() {
            ONE_TIME_INITIALIZE_THREAD_LOCAL(bool, GInstance, false);
            return GInstance;
        }
    };

    struct FBlockTracker {
        std::atomic<size_t> NumAllocs;

        class FBlockCompressedRadixTrie : public FCompressedRadixTrie {
        public:
#if USE_PPE_MEMORYDOMAINS
            FBlockCompressedRadixTrie() : FCompressedRadixTrie(MEMORYDOMAIN_TRACKING_DATA(LeakDetector))
#else
            FBlockCompressedRadixTrie()
#endif
            {}
        };

        STATIC_CONST_INTEGRAL(size_t, NumBuckets, 256 / ALLOCATION_BOUNDARY);
        FBlockCompressedRadixTrie Buckets[NumBuckets];

        FBlockTracker() : NumAllocs(0) {}
        ~FBlockTracker() {}

        FCompressedRadixTrie& PtrToBucket(void* ptr, uintptr_t* key) {
            Assert(ptr);
            Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, ptr));
            *key = (uintptr_t(ptr) & ~uintptr_t(0xFF));
            const size_t b = ((uintptr_t(ptr) & 0xFF) / ALLOCATION_BOUNDARY);
            Assert(b < NumBuckets);
            return Buckets[b];
        }

        void Allocate(void* ptr, const FBlockHeader& header) {
            NumAllocs++;
            uintptr_t key;
            FCompressedRadixTrie& bucket = PtrToBucket(ptr, &key);
            bucket.Insert(key, header.Pack());
        }

        const FBlockHeader& Fetch(void* ptr) {
            Assert(NumAllocs);
            uintptr_t key;
            FCompressedRadixTrie& bucket = PtrToBucket(ptr, &key);
            return FBlockHeader::Unpack(bucket.Lookup(key));
        }

        FBlockHeader Release(void* ptr) {
            Assert(NumAllocs);
            NumAllocs--;
            uintptr_t key;
            FCompressedRadixTrie& bucket = PtrToBucket(ptr, &key);
            return FBlockHeader::Unpack(bucket.Erase(key));
        }

        template <typename _Functor>
        void Foreach(_Functor&& foreach) {
            forrange(b, 0, NumBuckets) {
                Buckets[b].Foreach([&foreach, b](uintptr_t key, uintptr_t value) {
                    foreach((void*)(key | (b * ALLOCATION_BOUNDARY)), FBlockHeader::Unpack(value));
                });
            }
        }
    };

    struct FCallstackTracker {
        FAtomicSpinLock Barrier;

        size_t NumCallstacks;

        FPlatformLowLevelIO::FHandle FileHandle;
        u64 NativeOffset;

        STATIC_CONST_INTEGRAL(size_t, WriteBufferCapacity, (2 * 1024 * 1024) / sizeof(FCallstackData)); // 2 mb <=> 8192 different callstacks
        STATIC_CONST_INTEGRAL(size_t, WriteBufferSizeInBytes, WriteBufferCapacity * sizeof(FCallstackData));
        STATIC_ASSERT((2 * 1024 * 1024) == WriteBufferSizeInBytes);
        size_t BufferOffset;
        FCallstackData* const WriteBuffer;

        STATIC_CONST_INTEGRAL(size_t, HashTableCapacity, (8 * 1024 * 1024) / sizeof(FCallstackHeader)); // 8 mb <=> 524288 different callstacks
        STATIC_CONST_INTEGRAL(size_t, HashTableSizeInBytes, HashTableCapacity * sizeof(FCallstackHeader));
        STATIC_ASSERT((8 * 1024 * 1024) == HashTableSizeInBytes);
        STATIC_ASSERT(Meta::IsPow2(HashTableCapacity));
        STATIC_CONST_INTEGRAL(size_t, HashTableMask, HashTableCapacity - 1);
        FCallstackHeader* const HashTable;

        FCallstackTracker()
            : NumCallstacks(0)
            , FileHandle(FPlatformLowLevelIO::InvalidHandle)
            , NativeOffset(0)
            , BufferOffset(0)
#if USE_PPE_MEMORYDOMAINS
            , WriteBuffer((FCallstackData*)FVirtualMemory::InternalAlloc(WriteBufferSizeInBytes, MEMORYDOMAIN_TRACKING_DATA(LeakDetector)))
            , HashTable((FCallstackHeader*)FVirtualMemory::InternalAlloc(HashTableSizeInBytes, MEMORYDOMAIN_TRACKING_DATA(LeakDetector)))
#else
            , WriteBuffer((FCallstackData*)FVirtualMemory::InternalAlloc(WriteBufferSizeInBytes))
            , HashTable((FCallstackHeader*)FVirtualMemory::InternalAlloc(HashTableSizeInBytes))
#endif
        {}

        ~FCallstackTracker() {
            Flush();

            // close the stream outside of the lock to avoid dead locking
            if (FPlatformLowLevelIO::InvalidHandle != FileHandle)
                FPlatformLowLevelIO::Close(FileHandle);
            FileHandle = FPlatformLowLevelIO::InvalidHandle;

#if USE_PPE_MEMORYDOMAINS
            FVirtualMemory::InternalFree(WriteBuffer, WriteBufferSizeInBytes, MEMORYDOMAIN_TRACKING_DATA(LeakDetector));
            FVirtualMemory::InternalFree(HashTable, HashTableSizeInBytes, MEMORYDOMAIN_TRACKING_DATA(LeakDetector));
#else
            FVirtualMemory::InternalFree(WriteBuffer, WriteBufferSizeInBytes);
            FVirtualMemory::InternalFree(HashTable, HashTableSizeInBytes);
#endif
        }

        void OpenStream() {
            AssertIsMainThread();
            Assert(FPlatformLowLevelIO::InvalidHandle == FileHandle);

            const FWString fname = FPlatformFile::MakeTemporaryFile(L"LeakDetector", L".bin");
            FileHandle = FPlatformLowLevelIO::Open(fname.data(), EOpenPolicy::ReadWritable, EAccessPolicy::Temporary | EAccessPolicy::Truncate_Binary);
        }

        void Flush() {
            Assert(FPlatformLowLevelIO::InvalidHandle != FileHandle);

            const FAtomicSpinLock::FScope scopeLock(Barrier);

            Flush_AssumeLocked();
        }

        void Fetch(u32 uid, FCallstackHeader* pheader, FCallstackData* pdata) {
            Assert(uid < HashTableCapacity);
            Assert(HashTable[uid].Fingerprint);
            Assert(pheader);
            Assert(pdata);

            const FAtomicSpinLock::FScope scopeLock(Barrier);

            Flush_AssumeLocked();

            const std::streamoff org = FPlatformLowLevelIO::Tell(FileHandle);
            FPlatformLowLevelIO::Seek(FileHandle, pheader->StreamOffset, ESeekOrigin::Begin);

            *pheader = HashTable[uid];
            Verify(FPlatformLowLevelIO::Read(FileHandle, pdata, sizeof(FCallstackData)) == sizeof(FCallstackData));
            Assert(pdata->MakeFingerprint() == pheader->Fingerprint);

            FPlatformLowLevelIO::Seek(FileHandle, org, ESeekOrigin::Begin);
            Assert(checked_cast<u64>(FPlatformLowLevelIO::Tell(FileHandle)) == NativeOffset);
        }

        bool IsNonDeleter(u32 uid) const {
            Assert(uid < HashTableCapacity);
            Assert(HashTable[uid].Fingerprint);

            return HashTable[uid].IsNonDeleter();
        }

        void Delete(u32 uid) {
            Assert(uid < HashTableCapacity);
            Assert(HashTable[uid].Fingerprint);

            HashTable[uid].KnownDeleter = 1;
        }

        void Trim(u32 uid) {
            Assert(uid < HashTableCapacity);
            Assert(HashTable[uid].Fingerprint);

            HashTable[uid].KnownTrimmer = 1;
        }

        u32 FindOrAdd(const FCallstackData& callstack) {
            const u64 fingerprint = callstack.MakeFingerprint();

            // first search without locking :

            u32 uid = u32(-1);
            forrange(i, 0, HashTableCapacity) {
                uid = checked_cast<u32>((fingerprint + i) & HashTableMask);
                const FCallstackHeader& header = HashTable[uid];
                if (header.Fingerprint == 0 || header.Fingerprint == fingerprint)
                    break;
            }

            // if absent, take the lock and insert
            return (Unlikely(fingerprint != HashTable[uid].Fingerprint)
                ? AddIFN_Locked(fingerprint, callstack)
                : uid );
        }

    private:
        NO_INLINE void Flush_AssumeLocked() {
            Assert(FPlatformLowLevelIO::InvalidHandle != FileHandle);
            Assert(checked_cast<u64>(FPlatformLowLevelIO::Tell(FileHandle)) == NativeOffset);

            if (BufferOffset) {
                Verify(FPlatformLowLevelIO::Write(FileHandle, WriteBuffer, BufferOffset * sizeof(WriteBuffer[0])));
                NativeOffset = checked_cast<u64>(FPlatformLowLevelIO::Tell(FileHandle));
                BufferOffset = 0;
            }
        }

        u64 Write_AssumeLocked(const FCallstackData& callstack) {
            Assert(BufferOffset < WriteBufferCapacity);

            const u64 streamOffset = (NativeOffset + sizeof(FCallstackData) * BufferOffset);

            NumCallstacks++;
            WriteBuffer[BufferOffset++] = callstack;

            if (BufferOffset == WriteBufferCapacity)
                Flush_AssumeLocked();
            Assert(BufferOffset < WriteBufferCapacity);

            return streamOffset;
        }

        NO_INLINE u32 AddIFN_Locked(u64 fingerprint, const FCallstackData& callstack) {
            const FAtomicSpinLock::FScope scopeLock(Barrier);

            u32 uid = u32(-1);
            forrange(i, 0, HashTableCapacity) {
                uid = checked_cast<u32>((fingerprint + i) & HashTableMask);
                const FCallstackHeader& header = HashTable[uid];
                if (header.Fingerprint == 0 || header.Fingerprint == fingerprint)
                    break;
            }

            FCallstackHeader& header = HashTable[uid];
            if (0 == header.Fingerprint) {
                Assert(0 == header.KnownDeleter);
                Assert(0 == header.KnownTrimmer);

                header.Fingerprint = fingerprint;
                header.StreamOffset = Write_AssumeLocked(callstack);
            }
            Assert(fingerprint == header.Fingerprint);

            return uid;
        }
    };

    bool _enabled;
    FBlockTracker _blocks;
    FCallstackTracker _callstacks;

    FLeakDetector()
        : _enabled(false)
    {}

    ~FLeakDetector() {
        Assert(not _enabled);
        FRecursiveLockTLS::Get() = true; // forbid further logging
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_MALLOC_LEAKDETECTOR
