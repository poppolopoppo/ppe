#pragma once

#include "Core_fwd.h"

// Memory Domains ON/OFF
#ifndef USE_PPE_MEMORYDOMAINS
#   if not (USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING) // %_NOCOMMIT%
#       define USE_PPE_MEMORYDOMAINS 1
#   else
#       define USE_PPE_MEMORYDOMAINS 0
#   endif
#endif

// Memory domains collapsing (less codegen)
#define WITH_PPE_MEMORYDOMAINS_COLLAPSING (USE_PPE_MEMORYDOMAINS && USE_PPE_PROFILING) // %_NOCOMMIT%
#define WITH_PPE_MEMORYDOMAINS_FULL_COLLAPSING (WITH_PPE_MEMORYDOMAINS_COLLAPSING && 0) // turn to 1 to collapse all domains to Used/ReservedMemory %_NOCOMMIT%

#if USE_PPE_MEMORYDOMAINS
#   define ARG0_IF_MEMORYDOMAINS(...) __VA_ARGS__
#   define ARGS_IF_MEMORYDOMAINS(...) COMMA __VA_ARGS__
#   define ONLY_IF_MEMORYDOMAINS(...) __VA_ARGS__
#else
#   define ARG0_IF_MEMORYDOMAINS(...)
#   define ARGS_IF_MEMORYDOMAINS(...)
#   define ONLY_IF_MEMORYDOMAINS(...) NOOP()
#endif

namespace PPE {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryTracking;
//----------------------------------------------------------------------------
#define MEMORYDOMAIN_NAME(_Name) CONCAT(F, _Name)
#define MEMORYDOMAIN_TAG(_Name) ::PPE::MemoryDomain::MEMORYDOMAIN_NAME(_Name)
#define MEMORYDOMAIN_TRACKING_DATA(_Name) MEMORYDOMAIN_TAG(_Name)::TrackingData()
//----------------------------------------------------------------------------
namespace MemoryDomain {
struct MEMORYDOMAIN_NAME(GpuMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
struct MEMORYDOMAIN_NAME(PooledMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
struct MEMORYDOMAIN_NAME(ReservedMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
struct MEMORYDOMAIN_NAME(UsedMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
struct MEMORYDOMAIN_NAME(VirtualMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
struct MEMORYDOMAIN_NAME(UnaccountedMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
}   // ^^^ don't use those directly ! always prefer explicit domains vvv
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
