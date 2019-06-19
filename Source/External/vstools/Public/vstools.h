#pragma once

#if defined(PLATFORM_WINDOWS) && !defined(FINAL_RELEASE)
#   define USE_PPE_EXTERNAL_VSTOOLS 1
#else
#   define USE_PPE_EXTERNAL_VSTOOLS 0
#endif

#if USE_PPE_EXTERNAL_VSTOOLS

#include "Meta/Aliases.h"

#ifdef EXPORT_PPE_EXTERNAL_VSTOOLS
#   define PPE_VSTOOLS_API DLL_EXPORT
#else
#   define PPE_VSTOOLS_API DLL_IMPORT
#endif

extern "C" {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef void* vstools_FHeapTracker;
//----------------------------------------------------------------------------
PPE_VSTOOLS_API vstools_FHeapTracker vstools_CreateHeapTracker(const char* name);
PPE_VSTOOLS_API void vstools_DestroyHeapTracker(vstools_FHeapTracker heap);
//----------------------------------------------------------------------------
PPE_VSTOOLS_API bool vstools_IsValidHeapTracker(vstools_FHeapTracker heap) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_VSTOOLS_API bool vstools_AllocateEvent(vstools_FHeapTracker heap, void* ptr, size_t sz);
PPE_VSTOOLS_API bool vstools_ReallocateEvent(vstools_FHeapTracker heap, void* newp, size_t newsz, void* oldp);
PPE_VSTOOLS_API bool vstools_DeallocateEvent(vstools_FHeapTracker heap, void* ptr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!extern "C"

#endif //!USE_PPE_EXTERNAL_VSTOOLS
