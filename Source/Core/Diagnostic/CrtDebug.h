#pragma once

#include "Core/Core.h"

#include "Core/Memory/UniquePtr.h"

#if defined(PLATFORM_WINDOWS) && defined(_DEBUG)
//#   define USE_CRT_DEBUG %_NOCOMMIT%
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CRT_DEBUG
//----------------------------------------------------------------------------
class FCrtCheckMemoryLeaksImpl;
//----------------------------------------------------------------------------
class FCrtCheckMemoryLeaks {
public:
    FCrtCheckMemoryLeaks();
    ~FCrtCheckMemoryLeaks();
private:
    TUniquePtr<FCrtCheckMemoryLeaksImpl> _pimpl;
};
//----------------------------------------------------------------------------
class FCrtSkipMemoryLeaks {
public:
    FCrtSkipMemoryLeaks();
    ~FCrtSkipMemoryLeaks();
private:
    bool _prev;
};
//----------------------------------------------------------------------------
void CrtCheckGlobalMemoryLeaks(bool enabled);
//----------------------------------------------------------------------------
#define CHECK_MEMORY_LEAKS_IN_SCOPE() \
    const ::Core::FCrtCheckMemoryLeaks ANONYMIZE(checkMemoryLeaksScope_)
//----------------------------------------------------------------------------
#define SKIP_MEMORY_LEAKS_IN_SCOPE() \
    const ::Core::FCrtSkipMemoryLeaks ANONYMIZE(skipMemoryLeaksScope_)
//----------------------------------------------------------------------------
#define GLOBAL_CHECK_MEMORY_LEAKS(_Enabled) \
    ::Core::CrtCheckGlobalMemoryLeaks(_Enabled)
//----------------------------------------------------------------------------
#else  // USE_CRT_DEBUG
//----------------------------------------------------------------------------
#define CHECK_MEMORY_LEAKS_IN_SCOPE() NOOP()
//----------------------------------------------------------------------------
#define SKIP_MEMORY_LEAKS_IN_SCOPE() NOOP()
//----------------------------------------------------------------------------
#define GLOBAL_CHECK_MEMORY_LEAKS(_Enabled) NOOP()
//----------------------------------------------------------------------------
#endif //!USE_CRT_DEBUG
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
//----------------------------------------------------------------------------
class FCrtMemoryStats {
public:
    size_t TotalUsedSize;
    size_t LargestUsedBlockSize;
    size_t TotalFreeSize;
    size_t LargestFreeBlockSize;
    size_t TotalOverheadSize;

    // http://en.wikipedia.org/wiki/Fragmentation_(computing)#External_fragmentation
    float ExternalFragmentation() const {
        return (0 == TotalFreeSize ? 0 : 1.0f - (float)LargestFreeBlockSize / TotalFreeSize);
    }
};
//----------------------------------------------------------------------------
bool CrtDumpMemoryStats(FCrtMemoryStats* memoryStats, void* heapHandle = nullptr);
//----------------------------------------------------------------------------
#endif //!PLATFORM_WINDOWS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
