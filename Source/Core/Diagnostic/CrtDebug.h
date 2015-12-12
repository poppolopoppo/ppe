#pragma once

#include "Core/Core.h"

#include "Core/IO/FormatHelpers.h"
#include "Core/Memory/UniquePtr.h"

#if defined(OS_WINDOWS) && defined(_DEBUG)
//#   define USE_CRT_DEBUG %_NOCOMMIT%
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CRT_DEBUG
//----------------------------------------------------------------------------
class CrtCheckMemoryLeaksImpl;
//----------------------------------------------------------------------------
class CrtCheckMemoryLeaks {
public:
    CrtCheckMemoryLeaks();
    ~CrtCheckMemoryLeaks();
private:
    UniquePtr<CrtCheckMemoryLeaksImpl> _pimpl;
};
//----------------------------------------------------------------------------
class CrtSkipMemoryLeaks {
public:
    CrtSkipMemoryLeaks();
    ~CrtSkipMemoryLeaks();
private:
    bool _prev;
};
//----------------------------------------------------------------------------
void CrtCheckGlobalMemoryLeaks(bool enabled);
//----------------------------------------------------------------------------
#define CHECK_MEMORY_LEAKS_IN_SCOPE() \
    const ::Core::CrtCheckMemoryLeaks CONCAT(checkMemoryLeaksScope_, __LINE__)
//----------------------------------------------------------------------------
#define SKIP_MEMORY_LEAKS_IN_SCOPE() \
    const ::Core::CrtSkipMemoryLeaks CONCAT(skipMemoryLeaksScope_, __LINE__)
//----------------------------------------------------------------------------
#define GLOBAL_CHECK_MEMORY_LEAKS(_Enabled) \
    ::Core::CrtCheckGlobalMemoryLeaks(_Enabled)
//----------------------------------------------------------------------------
#else  // USE_CRT_DEBUG
//----------------------------------------------------------------------------
#define CHECK_MEMORY_LEAKS_IN_SCOPE() NOOP
//----------------------------------------------------------------------------
#define SKIP_MEMORY_LEAKS_IN_SCOPE() NOOP
//----------------------------------------------------------------------------
#define GLOBAL_CHECK_MEMORY_LEAKS(_Enabled) NOOP
//----------------------------------------------------------------------------
#endif //!USE_CRT_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef OS_WINDOWS
//----------------------------------------------------------------------------
class CrtMemoryStats {
public:
    SizeInBytes TotalUsedSize;
    SizeInBytes LargestUsedBlockSize;
    SizeInBytes TotalFreeSize;
    SizeInBytes LargestFreeBlockSize;
    SizeInBytes TotalOverheadSize;

    // http://en.wikipedia.org/wiki/Fragmentation_(computing)#External_fragmentation
    float ExternalFragmentation() const {
        return (0 == TotalFreeSize.Value) ? 0 : 1.0f - (float)LargestFreeBlockSize.Value / TotalFreeSize.Value;
    }
};
//----------------------------------------------------------------------------
bool CrtDumpMemoryStats(CrtMemoryStats* memoryStats, void* heapHandle = nullptr);
//----------------------------------------------------------------------------
#define BREAKPOINT() _CrtDbgBreak()
//----------------------------------------------------------------------------
#else  // OS_WINDOWS
//----------------------------------------------------------------------------
#define BREAKPOINT()                        NOOP
//----------------------------------------------------------------------------
#endif //!OS_WINDOWS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
