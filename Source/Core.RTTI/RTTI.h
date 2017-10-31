#pragma once

#include "Core/Core.h"

#ifdef EXPORT_CORE_RTTI
#   define CORE_RTTI_API DLL_EXPORT
#else
#   define CORE_RTTI_API DLL_IMPORT
#endif

#if !defined(FINAL_RELEASE) && !defined(PROFILING_ENABLED)
#   define USE_CORE_RTTI_CHECKS 1
#else
#   define USE_CORE_RTTI_CHECKS 0
#endif

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_RTTI_API FRTTIModule {
public:
    static void Start();
    static void Shutdown();

    static void Clear();
    static void ClearAll_UnusedMemory();

    FRTTIModule() { Start(); }
    ~FRTTIModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
