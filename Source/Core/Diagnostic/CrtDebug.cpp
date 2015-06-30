#include "stdafx.h"

#include "CrtDebug.h"

#if defined(WIN32)
#include "Callstack.h"
#include "DecodedCallstack.h"
#include "Logger.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringSlice.h"

#include <iostream>
#include <crtdbg.h>
#include <Windows.h>

#include <thread>
#include <mutex>
#include <unordered_map>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef _DEBUG
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static THREAD_LOCAL bool gEnableCrtAllocationHook = true;
//----------------------------------------------------------------------------
class CrtAllocationCallstackLogger {
public:
    struct TAllocation {
        enum { MaxDepth = 31 };
        size_t RequestNumber;
        void* Backtrace[MaxDepth];
    };

    struct TPoolChunk {
        enum { ChunkSize = (4*1024*1024/*4mo*/-sizeof(size_t))/sizeof(TAllocation) };
        TAllocation Allocations[ChunkSize];
        TPoolChunk* Next;
    };

    explicit CrtAllocationCallstackLogger();
    ~CrtAllocationCallstackLogger();

    void Allocate(long requestNumber);
    const TAllocation* Callstack(long requestNumber) const;

    size_t AllocationCount() const { return _allocationCount; }
    size_t ChunkCount() const { return _chunkCount; }
    SizeInBytes TotalSizeInBytes() const { return SizeInBytes{ _chunkCount * sizeof(TPoolChunk) }; }

    static _declspec(dllexport) CrtAllocationCallstackLogger* Instance;

protected:
    bool NeedNewPoolChunk_() const;
    void AllocatePoolChunk_();
    void DestroyPoolChunks_();

    static const TAllocation* FindRequestAllocation_(
        size_t requestNumber,
        const TAllocation* begin,
        const TAllocation* end
        );

private:
    mutable std::mutex _barrier;
    std::atomic<bool> _enabled;
    size_t _chunkCount;
    size_t _allocationCount;
    TPoolChunk* _chunks;
    TAllocation* _freeAllocation;
};
//----------------------------------------------------------------------------
CrtAllocationCallstackLogger* CrtAllocationCallstackLogger::Instance = nullptr;
//----------------------------------------------------------------------------
CrtAllocationCallstackLogger::CrtAllocationCallstackLogger()
:   _enabled(true),
    _chunkCount(0), _allocationCount(0),
    _freeAllocation(nullptr), _chunks(nullptr) {
    AllocatePoolChunk_();
}
//----------------------------------------------------------------------------
CrtAllocationCallstackLogger::~CrtAllocationCallstackLogger() {
    _enabled = false;
    DestroyPoolChunks_();
}
//----------------------------------------------------------------------------
void CrtAllocationCallstackLogger::Allocate(long requestNumber) {
    if (!_enabled) // filter internal allocations
        return;

    _enabled = false;
    std::lock_guard<std::mutex> scope_lock(_barrier);

    if (NeedNewPoolChunk_())
        AllocatePoolChunk_();

    TAllocation* const alloc = _freeAllocation;
    ++_freeAllocation;
    ++_allocationCount;

    alloc->RequestNumber = requestNumber;

    const size_t depth = Core::Callstack::Capture(MakeView(alloc->Backtrace), nullptr, 6, TAllocation::MaxDepth);
    if (depth < TAllocation::MaxDepth)
        alloc->Backtrace[depth] = nullptr;

    _enabled = true;
}
//----------------------------------------------------------------------------
auto CrtAllocationCallstackLogger::Callstack(long requestNumber) const -> const TAllocation*{
    std::lock_guard<std::mutex> scope_lock(_barrier);

    if (nullptr == _chunks)
        return nullptr;

    const TAllocation* result = nullptr;
    if (_freeAllocation > &_chunks->Allocations[0] &&
        nullptr != (result = FindRequestAllocation_(requestNumber, &_chunks->Allocations[0], _freeAllocation)) )
        return result;

    for (const TPoolChunk* chunk = _chunks->Next; chunk; chunk = chunk->Next)
        if (nullptr != (result = FindRequestAllocation_(requestNumber, &chunk->Allocations[0], &chunk->Allocations[TPoolChunk::ChunkSize])) )
            return result;

    return nullptr;
}
//----------------------------------------------------------------------------
bool CrtAllocationCallstackLogger::NeedNewPoolChunk_() const {
    return  nullptr == _freeAllocation ||
            &_chunks->Allocations[lengthof(_chunks->Allocations)] == _freeAllocation;
}
//----------------------------------------------------------------------------
void CrtAllocationCallstackLogger::AllocatePoolChunk_() {
    TPoolChunk* const new_chunk = (TPoolChunk*)VirtualAlloc(
        nullptr,
        sizeof(TPoolChunk),
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
        );

    new_chunk->Next = _chunks;
    _chunks = new_chunk;
    _freeAllocation = new_chunk->Allocations;
    ++_chunkCount;
}
//----------------------------------------------------------------------------
void CrtAllocationCallstackLogger::DestroyPoolChunks_() {
    _freeAllocation = nullptr;
    while (_chunks) {
        TPoolChunk* const next = _chunks->Next;
        //_free_dbg(_chunks, _CRT_BLOCK);
        VirtualFree(_chunks, 0, MEM_RELEASE);
        _chunks = next;
    }
}
//----------------------------------------------------------------------------
auto CrtAllocationCallstackLogger::FindRequestAllocation_(
    size_t requestNumber,
    const TAllocation* begin,
    const TAllocation* end
    ) -> const TAllocation* {
    if (begin->RequestNumber > requestNumber ||
        (end - 1)->RequestNumber < requestNumber)
        return nullptr;

    do {
        const TAllocation* pivot = begin + (end - begin) / 2;
        if (pivot->RequestNumber == requestNumber)
            return pivot;
        else if (pivot->RequestNumber > requestNumber)
            end = pivot;
        else
            begin = pivot;
    } while (begin < end);

    return nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
#endif //!_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef _DEBUG
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

    auto const log = CrtAllocationCallstackLogger::Instance;

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

    if (_CRT_WARN != nRptType ||
        nullptr == szMsg)
        return TRUE;

    if ('{' == *szMsg)
    {
        size_t i = 0;
        while (szMsg[i + 1] != '\0' && szMsg[i + 1] != '}')
            ++i;

        long requestNumber = 0;
        for (int n = 1; i > 0; --i, n *= 10)
            requestNumber += n * (szMsg[i] - '0');

        auto const log = CrtAllocationCallstackLogger::Instance;
        if (log) {
            const CrtAllocationCallstackLogger::TAllocation* alloc = log->Callstack(requestNumber);
            if (alloc)
            {
                size_t depth = 0;
                for (; depth < CrtAllocationCallstackLogger::TAllocation::MaxDepth && alloc->Backtrace[depth]; ++depth)
                    ;

                DecodedCallstack decoded;
                Core::Callstack::Decode(&decoded, 0, MakeView(&alloc->Backtrace[0], &alloc->Backtrace[depth]));

                LOG(Error, L"Error leak detected #{0} !", alloc->RequestNumber);
                for (const DecodedCallstack::Frame& frame : decoded.Frames())
                    LOG(Callstack, L"{0}", frame);
            }
        }
    }
    else {
        StringSlice msgWithoutEndl;
        const char *p = szMsg;
        if (Split(&p, "\n", msgWithoutEndl))
            LOG(Error, L"{0}", msgWithoutEndl);
    }

    return (TRUE); // Allow the memory operation to proceed
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
#endif //!_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef _DEBUG
//----------------------------------------------------------------------------
class CrtCheckMemoryLeaksImpl {
public:
    CrtCheckMemoryLeaksImpl();
    ~CrtCheckMemoryLeaksImpl();
private:
    _CRT_ALLOC_HOOK _allocHook;
    _CrtMemState _memCheckpoint;
    CrtAllocationCallstackLogger _log;
};
//----------------------------------------------------------------------------
CrtCheckMemoryLeaksImpl::CrtCheckMemoryLeaksImpl()
    : _log() {
    CrtAllocationCallstackLogger::Instance = &_log;
    _CrtMemCheckpoint(&_memCheckpoint);
    _allocHook = _CrtSetAllocHook(AllocHook_);
}
//----------------------------------------------------------------------------
CrtCheckMemoryLeaksImpl::~CrtCheckMemoryLeaksImpl() {
    _CrtSetAllocHook(_allocHook);

    _CrtMemState memCurrent;
    _CrtMemCheckpoint(&memCurrent);
    _CrtMemState memDiff;
    if (!_CrtMemDifference(&memDiff, &_memCheckpoint, &memCurrent))
        return; // no diff => bye, bye

    const _CRT_REPORT_HOOK reportHook = _CrtSetReportHook(ReportHook_);
    _CrtMemDumpAllObjectsSince(&_memCheckpoint);
    _CrtSetReportHook(reportHook);

    Assert(CrtAllocationCallstackLogger::Instance == &_log);
    CrtAllocationCallstackLogger::Instance = nullptr;

    LOG(Information, L"Allocation logger overhead = {0} ({1})", _log.TotalSizeInBytes(), _log.ChunkCount());
    LOG(Information, L"Total allocation count = {0}", _log.AllocationCount());

    _CrtMemDumpStatistics(&memDiff);
    _CrtCheckMemory();
}
//----------------------------------------------------------------------------
#endif //_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef _DEBUG
//----------------------------------------------------------------------------
CrtCheckMemoryLeaks::CrtCheckMemoryLeaks()
:   _pimpl(new CrtCheckMemoryLeaksImpl()) {}
//----------------------------------------------------------------------------
CrtCheckMemoryLeaks::~CrtCheckMemoryLeaks() {}
//----------------------------------------------------------------------------
#endif //!_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef _DEBUG
//----------------------------------------------------------------------------
CrtSkipMemoryLeaks::CrtSkipMemoryLeaks() {
    _prev = gEnableCrtAllocationHook;
    gEnableCrtAllocationHook = false;
}
//----------------------------------------------------------------------------
CrtSkipMemoryLeaks::~CrtSkipMemoryLeaks() {
    gEnableCrtAllocationHook = _prev;
}
//----------------------------------------------------------------------------
#endif //!_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef _DEBUG
//----------------------------------------------------------------------------
static std::unique_ptr<CrtCheckMemoryLeaksImpl> gGlobalCrtCheckMemoryLeaks;
//----------------------------------------------------------------------------
void CrtCheckGlobalMemoryLeaks(bool enabled) {
    if (enabled) {
        Assert(nullptr == gGlobalCrtCheckMemoryLeaks.get());
        gGlobalCrtCheckMemoryLeaks.reset(new CrtCheckMemoryLeaksImpl());
    }
    else {
        Assert(nullptr != gGlobalCrtCheckMemoryLeaks.get());
        gGlobalCrtCheckMemoryLeaks.reset();
    }
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
void CrtCheckGlobalMemoryLeaks(bool ) {}
//----------------------------------------------------------------------------
#endif //!_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool CrtDumpMemoryStats(CrtMemoryStats* memoryStats, void* heapHandle/* = nullptr */) {
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
#endif //!#if defined(WIN32)
