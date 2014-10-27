#pragma once

#include "Core/Core.h"

#include "Core/IO/FormatHelpers.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
#ifdef WIN32
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CrtCheckMemoryLeaksImpl;
//----------------------------------------------------------------------------
class CrtCheckMemoryLeaks {
#ifdef _DEBUG
public:
    CrtCheckMemoryLeaks();
    ~CrtCheckMemoryLeaks();
private:
    UniquePtr<CrtCheckMemoryLeaksImpl> _pimpl;
#endif
};
//----------------------------------------------------------------------------
class CrtSkipMemoryLeaks {
#ifdef _DEBUG
public:
    CrtSkipMemoryLeaks();
    ~CrtSkipMemoryLeaks();
private:
    bool _prev;
#endif
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
//////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define BREAKPOINT() _CrtDbgBreak()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#else //WIN32
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CHECK_MEMORY_LEAKS_IN_SCOPE()   NOOP
//----------------------------------------------------------------------------
#define SKIP_MEMORY_LEAKS_IN_SCOPE()    NOOP
//----------------------------------------------------------------------------
#define GLOBAL_CHECK_MEMORY_LEAKS(_Enabled) NOOP
//----------------------------------------------------------------------------
#define BREAKPOINT()                    NOOP
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#endif //!WIN32
} //!namespace Core
