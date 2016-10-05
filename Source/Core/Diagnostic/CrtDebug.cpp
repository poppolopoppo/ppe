#include "stdafx.h"

#include "CrtDebug.h"

#ifdef OS_WINDOWS

#include "Callstack.h"
#include "DecodedCallstack.h"
#include "Logger.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringView.h"

#include "Thread/ReadWriteLock.h"

#include <iostream>
#include <crtdbg.h>
#include <Windows.h>

#include <thread>
#include <unordered_map>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CRT_DEBUG
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static THREAD_LOCAL bool gEnableCrtAllocationHook = true;
//----------------------------------------------------------------------------
class FCrtAllocationCallstackLogger {
public:
    struct FTAllocation {
        enum { MaxDepth = 31 };
        size_t RequestNumber;
        void* Backtrace[MaxDepth];
    };

    struct FTPoolChunk {
        enum { ChunkSize = (4*1024*1024/*4mo*/-sizeof(size_t))/sizeof(FTAllocation) };
        FTAllocation Allocations[ChunkSize];
        FTPoolChunk* Next;
    };

    explicit FCrtAllocationCallstackLogger();
    ~FCrtAllocationCallstackLogger();

    void Allocate(long requestNumber);
    const FTAllocation* FCallstack(long requestNumber) const;

    size_t AllocationCount() const { return _allocationCount; }
    size_t ChunkCount() const { return _chunkCount; }
    SizeInBytes TotalSizeInBytes() const { return SizeInBytes{ _chunkCount * sizeof(FTPoolChunk) }; }

    static FCrtAllocationCallstackLogger* Instance;

protected:
    bool NeedNewPoolChunk_() const;
    void AllocatePoolChunk_();
    void DestroyPoolChunks_();

    static const FTAllocation* FindRequestAllocation_(
        size_t requestNumber,
        const FTAllocation* begin,
        const FTAllocation* end
        );

private:
    FReadWriteLock _barrier;
    std::atomic<bool> _enabled;
    size_t _chunkCount;
    size_t _allocationCount;
    FTPoolChunk* _chunks;
    FTAllocation* _freeAllocation;
};
//----------------------------------------------------------------------------
FCrtAllocationCallstackLogger* FCrtAllocationCallstackLogger::Instance = nullptr;
//----------------------------------------------------------------------------
FCrtAllocationCallstackLogger::FCrtAllocationCallstackLogger()
:   _enabled(true),
    _chunkCount(0), _allocationCount(0),
    _freeAllocation(nullptr), _chunks(nullptr) {
    AllocatePoolChunk_();
}
//----------------------------------------------------------------------------
FCrtAllocationCallstackLogger::~FCrtAllocationCallstackLogger() {
    _enabled = false;
    DestroyPoolChunks_();
}
//----------------------------------------------------------------------------
void FCrtAllocationCallstackLogger::Allocate(long requestNumber) {
    if (!_enabled) // filter internal allocations
        return;

    _enabled = false;
    WRITESCOPELOCK(_barrier);

    if (NeedNewPoolChunk_())
        AllocatePoolChunk_();

    FTAllocation* const alloc = _freeAllocation;
    ++_freeAllocation;
    ++_allocationCount;

    alloc->RequestNumber = requestNumber;

    const size_t depth = Core::FCallstack::Capture(MakeView(alloc->Backtrace), nullptr, 6, FTAllocation::MaxDepth);
    if (depth < FTAllocation::MaxDepth)
        alloc->Backtrace[depth] = nullptr;

    _enabled = true;
}
//----------------------------------------------------------------------------
auto FCrtAllocationCallstackLogger::FCallstack(long requestNumber) const -> const FTAllocation*{
    READSCOPELOCK(_barrier);

    if (nullptr == _chunks)
        return nullptr;

    const FTAllocation* result = nullptr;
    if (_freeAllocation > &_chunks->Allocations[0] &&
        nullptr != (result = FindRequestAllocation_(requestNumber, &_chunks->Allocations[0], _freeAllocation)) )
        return result;

    for (const FTPoolChunk* chunk = _chunks->Next; chunk; chunk = chunk->Next)
        if (nullptr != (result = FindRequestAllocation_(requestNumber, &chunk->Allocations[0], &chunk->Allocations[FTPoolChunk::ChunkSize])) )
            return result;

    return nullptr;
}
//----------------------------------------------------------------------------
bool FCrtAllocationCallstackLogger::NeedNewPoolChunk_() const {
    return  nullptr == _freeAllocation ||
            &_chunks->Allocations[lengthof(_chunks->Allocations)] == _freeAllocation;
}
//----------------------------------------------------------------------------
void FCrtAllocationCallstackLogger::AllocatePoolChunk_() {
    FTPoolChunk* const new_chunk = (FTPoolChunk*)VirtualAlloc(
        nullptr,
        sizeof(FTPoolChunk),
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
        );

    new_chunk->Next = _chunks;
    _chunks = new_chunk;
    _freeAllocation = new_chunk->Allocations;
    ++_chunkCount;
}
//----------------------------------------------------------------------------
void FCrtAllocationCallstackLogger::DestroyPoolChunks_() {
    _freeAllocation = nullptr;
    while (_chunks) {
        FTPoolChunk* const next = _chunks->Next;
        //_free_dbg(_chunks, _CRT_BLOCK);
        VirtualFree(_chunks, 0, MEM_RELEASE);
        _chunks = next;
    }
}
//----------------------------------------------------------------------------
auto FCrtAllocationCallstackLogger::FindRequestAllocation_(
    size_t requestNumber,
    const FTAllocation* begin,
    const FTAllocation* end
    ) -> const FTAllocation* {
    Assert(begin <= end);

    if (begin->RequestNumber > requestNumber ||
        (end - 1)->RequestNumber < requestNumber)
        return nullptr;

     while (begin + 1 < end) {
        const FTAllocation* pivot = begin + ((end - begin - 1) >> 1);
        Assert(pivot < end);
        if (pivot->RequestNumber < requestNumber)
            begin = pivot + 1;
        else
            end = pivot + 1;
    }

    return ((begin + 1 == end) && (begin->RequestNumber == requestNumber))
        ? begin : nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
#endif //!USE_CRT_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CRT_DEBUG
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
/* CLIENT DUMP HOOK FUNCTION
//----------------------------------------------------------------------------
A hook function for dumping a Client block usually reports some
or all of the contents of the block in question.
*/
/*
static void __cdecl DumpClientHook_(void *pUserData, size_t nBytes) {
    NOOP;
}
*/
//----------------------------------------------------------------------------
/* ALLOCATION HOOK FUNCTION
//----------------------------------------------------------------------------
An allocation hook function can have many, many different
uses.
*/
static int __cdecl AllocHook_(
    int nAllocType,
    void* /* pvData */,
    size_t /* nSize */,
    int nBlockUse,
    long lRequest,
    const unsigned char * /* szFileName */,
    int /* nLine */) {
    if (nBlockUse == _CRT_BLOCK)
        return (TRUE);

    if (!gEnableCrtAllocationHook)
        return (TRUE);

    auto const log = FCrtAllocationCallstackLogger::Instance;

    switch (nAllocType)
    {
    case _HOOK_ALLOC:
    case _HOOK_REALLOC:
        log->Allocate(lRequest);
        break;
    case _HOOK_FREE:
        //log->Deallocate(lRequest); // lRequest is not accessible here ... FAIL
        break;
    }

    return (TRUE); // Allow the memory operation to proceed
}
//----------------------------------------------------------------------------
/* REPORT HOOK FUNCTION
//----------------------------------------------------------------------------
Again, report hook functions can serve a very wide variety of purposes.
This one logs error and assertion failure debug reports in the
log file, along with 'Damage' reports about overwritten memory.

By setting the retVal parameter to zero, we are instructing _CrtDbgReport
to return zero, which causes execution to continue. If we want the function
to start the debugger, we should have _CrtDbgReport return one.
*/
static int __cdecl ReportHook_(int nRptType, char *szMsg, int *retVal) {
    Assert(szMsg);
    Assert(retVal);

    if (_CRT_WARN != nRptType || nullptr == szMsg)
        return TRUE;

    if ('{' == *szMsg)
    {
        size_t i = 0;
        while (szMsg[i + 1] != '\0' && szMsg[i + 1] != '}')
            ++i;

        long requestNumber = 0;
        for (int n = 1; i > 0; --i, n *= 10)
            requestNumber += n * (szMsg[i] - '0');

        auto const* log = FCrtAllocationCallstackLogger::Instance;
        if (log) {
            const FCrtAllocationCallstackLogger::FTAllocation* alloc = log->Callstack(requestNumber);
            if (alloc)
            {
                size_t depth = 0;
                for (; depth < FCrtAllocationCallstackLogger::FTAllocation::MaxDepth && alloc->Backtrace[depth]; ++depth)
                    ;

                FDecodedCallstack decoded;
                Core::FCallstack::Decode(&decoded, 0, MakeView(&alloc->Backtrace[0], &alloc->Backtrace[depth]));

                LOG(Error, L"Error leak detected #{0} !\n{1}", alloc->RequestNumber, decoded);
            }
        }
    }
    else {
        FStringView msgWithoutEndl;
        FStringView msg = MakeStringView(szMsg, Meta::noinit_tag{});
        if (Split(msg, "\n", msgWithoutEndl))
            LOG(Error, L"{0}", msgWithoutEndl);
    }

    return (TRUE); // Allow the memory operation to proceed
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
#endif //!USE_CRT_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CRT_DEBUG
//----------------------------------------------------------------------------
class FCrtCheckMemoryLeaksImpl {
public:
    FCrtCheckMemoryLeaksImpl();
    ~FCrtCheckMemoryLeaksImpl();
private:
    _CRT_ALLOC_HOOK _allocHook;
    _CrtMemState _memCheckpoint;
    FCrtAllocationCallstackLogger _log;
};
//----------------------------------------------------------------------------
FCrtCheckMemoryLeaksImpl::FCrtCheckMemoryLeaksImpl()
    : _log() {
    FCrtAllocationCallstackLogger::Instance = &_log;
    _CrtMemCheckpoint(&_memCheckpoint);
    _allocHook = _CrtSetAllocHook(AllocHook_);
}
//----------------------------------------------------------------------------
FCrtCheckMemoryLeaksImpl::~FCrtCheckMemoryLeaksImpl() {
    _CrtSetAllocHook(_allocHook);

    _CrtMemState memCurrent;
    _CrtMemCheckpoint(&memCurrent);
    _CrtMemState memDiff;
    if (FALSE == _CrtMemDifference(&memDiff, &_memCheckpoint, &memCurrent))
        return; // no diff => bye, bye

    const _CRT_REPORT_HOOK reportHook = _CrtSetReportHook(ReportHook_);
    _CrtMemDumpAllObjectsSince(&_memCheckpoint);
    _CrtSetReportHook(reportHook);

    Assert(FCrtAllocationCallstackLogger::Instance == &_log);
    FCrtAllocationCallstackLogger::Instance = nullptr;

    LOG(Info, L"Allocation logger overhead = {0} ({1})", _log.TotalSizeInBytes(), _log.ChunkCount());
    LOG(Info, L"Total allocation count = {0}", _log.AllocationCount());

    _CrtMemDumpStatistics(&memDiff);
    _CrtCheckMemory();
}
//----------------------------------------------------------------------------
#endif //!USE_CRT_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CRT_DEBUG
//----------------------------------------------------------------------------
FCrtCheckMemoryLeaks::FCrtCheckMemoryLeaks()
:   _pimpl(new FCrtCheckMemoryLeaksImpl()) {}
//----------------------------------------------------------------------------
FCrtCheckMemoryLeaks::~FCrtCheckMemoryLeaks() {}
//----------------------------------------------------------------------------
#endif //!USE_CRT_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CRT_DEBUG
//----------------------------------------------------------------------------
FCrtSkipMemoryLeaks::FCrtSkipMemoryLeaks() {
    _prev = gEnableCrtAllocationHook;
    gEnableCrtAllocationHook = false;
}
//----------------------------------------------------------------------------
FCrtSkipMemoryLeaks::~FCrtSkipMemoryLeaks() {
    gEnableCrtAllocationHook = _prev;
}
//----------------------------------------------------------------------------
#endif //!USE_CRT_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CRT_DEBUG
//----------------------------------------------------------------------------
static std::unique_ptr<FCrtCheckMemoryLeaksImpl> gGlobalCrtCheckMemoryLeaks;
//----------------------------------------------------------------------------
void CrtCheckGlobalMemoryLeaks(bool enabled) {
    if (enabled) {
        Assert(nullptr == gGlobalCrtCheckMemoryLeaks.get());
        gGlobalCrtCheckMemoryLeaks.reset(new FCrtCheckMemoryLeaksImpl());
    }
    else {
        Assert(nullptr != gGlobalCrtCheckMemoryLeaks.get());
        gGlobalCrtCheckMemoryLeaks.reset();
    }
}
//----------------------------------------------------------------------------
#endif //!USE_CRT_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool CrtDumpMemoryStats(FCrtMemoryStats* memoryStats, void* heapHandle/* = nullptr */) {
    Assert(memoryStats);

    // walk default process heap if not supplied
    if (nullptr == heapHandle)
        heapHandle = GetProcessHeap();

    size_t totalFreeSize = 0;
    size_t largestFreeBlockSize = 0;
    size_t totalUsedSize = 0;
    size_t largestUsedBlockSize = 0;
    size_t totalOverheadSize = 0;

    PROCESS_HEAP_ENTRY entry;
    entry.lpData = nullptr;

    if (!HeapLock(heapHandle))
        return false;

    while (HeapWalk(heapHandle, &entry)) {
        if (PROCESS_HEAP_UNCOMMITTED_RANGE & entry.wFlags) {
            totalFreeSize += entry.cbData;
            if (largestFreeBlockSize < entry.cbData)
                largestFreeBlockSize = entry.cbData;
        }
        else if (PROCESS_HEAP_ENTRY_BUSY & entry.wFlags) {
            totalUsedSize += entry.cbData;
            if (largestUsedBlockSize < entry.cbData)
                largestUsedBlockSize = entry.cbData;
        }
        totalOverheadSize += entry.cbOverhead;
    }
    //Assert(ERROR_NO_MORE_ITEMS == GetLastError());

    if (!HeapUnlock(heapHandle))
    {
        Assert(false); // we obtained the lock though, bro ...
        return false;
    }

    memoryStats->TotalFreeSize = SizeInBytes{ totalFreeSize };
    memoryStats->LargestFreeBlockSize = SizeInBytes{ largestFreeBlockSize };
    memoryStats->TotalUsedSize = SizeInBytes{ totalUsedSize };
    memoryStats->LargestUsedBlockSize = SizeInBytes{ largestUsedBlockSize };
    memoryStats->TotalOverheadSize = SizeInBytes{ totalOverheadSize };

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!OS_WINDOWS
