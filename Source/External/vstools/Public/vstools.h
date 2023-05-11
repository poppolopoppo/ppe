#pragma once

#if defined(PLATFORM_WINDOWS) && !defined(CPP_CLANG) && defined(_DEBUG) && !defined(NDEBUG) && !defined(FINAL_RELEASE) && !defined(USE_PPE_PROFILING)
#   define USE_PPE_EXTERNAL_VSTOOLS 1
#else
#   define USE_PPE_EXTERNAL_VSTOOLS 0
#endif

#include "Meta/Aliases.h"

#ifdef EXPORT_PPE_EXTERNAL_VSTOOLS
#   define PPE_VSTOOLS_API DLL_EXPORT
#else
#   define PPE_VSTOOLS_API DLL_IMPORT
#endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_VSTOOLS_API IVSTools {
public:
    typedef void* FHeapTracker;

    virtual bool Available() const NOEXCEPT = 0;

    virtual FHeapTracker CreateHeapTracker(const char* name) = 0;
    virtual void DestroyHeapTracker(FHeapTracker heap) = 0;
    virtual bool IsValidHeapTracker(FHeapTracker heap) const NOEXCEPT = 0;

    virtual bool AllocateEvent(FHeapTracker heap, void* ptr, size_t sz) = 0;
    virtual bool ReallocateEvent(FHeapTracker heap, void* newp, size_t newsz, void* oldp) = 0;
    virtual bool DeallocateEvent(FHeapTracker heap, void* ptr) = 0;
};
//----------------------------------------------------------------------------
extern "C" {
PPE_VSTOOLS_API IVSTools* GetVSToolsInterface() NOEXCEPT;
} //!extern "C"
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
