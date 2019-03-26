#pragma once

#include "Core_fwd.h"

#define USE_PPE_STACKMARKER (0 && !USE_PPE_FINAL_RELEASE) //%_NOCOMMIT%

#if (!USE_PPE_STACKMARKER)

#   define PPE_STACKMARKER(_CAPTION) NOOP()

#else

#   include "HAL/PlatformMemory.h"

#   define USE_PPE_STACKMARKER_LIGHTWEIGHT 1

#   if USE_PPE_STACKMARKER_LIGHTWEIGHT
#       define PPE_STACKMARKER(_CAPTION) PPE::FStackMarker ANONYMIZE(_stackMarker){ \
        PPE::FPlatformMemory::AddressOfReturnAddress(), \
        _CAPTION, \
        nullptr, nullptr, 0 }
#   else
#       define PPE_STACKMARKER(_CAPTION) PPE::FStackMarker ANONYMIZE(_stackMarker){ \
        PPE::FPlatformMemory::AddressOfReturnAddress(), \
        _CAPTION, \
        PPE_PRETTY_FUNCTION, \
        __FILE__, u32(__LINE__) }
#   endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FThreadStackInfo {
    u32 Depth;
    u32 Offset;
    u32 MaxSize;
    FPlatformMemory::FStackUsage Usage;
};
//----------------------------------------------------------------------------
PPE_CORE_API FThreadStackInfo ThreadStackInfo() NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API u32 StackMarkerBegin(
    void* rsp,
    const char* caption,
    const char* function,
    const char* filename,
    u32 line ) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API void StackMarkerEnd(void* rsp, u32 depth) NOEXCEPT;
//----------------------------------------------------------------------------
struct FStackMarker {
    void* const RSP;
    const u32 Depth;

    FStackMarker(
        void* rsp,
        const char* caption,
        const char* function,
        const char* filename,
        u32 line)
    :   RSP(rsp)
    ,   Depth(StackMarkerBegin(RSP, caption, function, filename, line))
    {}

    ~FStackMarker() {
        StackMarkerEnd(RSP, Depth);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_STACKMARKER
