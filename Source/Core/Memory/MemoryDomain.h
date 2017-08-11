#pragma once

#include "Core/Core.h"

// Memory Domains ON/OFF
#ifndef FINAL_RELEASE
#   define USE_MEMORY_DOMAINS
#endif

// Memory domains collapsing (less codegen)
#if defined(USE_MEMORY_DOMAINS) && defined(PROFILING_ENABLED) // %_NOCOMMIT%
#   define COLLAPSE_MEMORY_DOMAINS
#endif

namespace Core {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryTrackingData;
//----------------------------------------------------------------------------
#define MEMORY_DOMAIN_NAME(_Name) CONCAT(_, _Name)
#define MEMORY_DOMAIN_TAG(_Name) Core::Domain::MEMORY_DOMAIN_NAME(_Name)
#define MEMORY_DOMAIN_TRACKING_DATA(_Name) MEMORY_DOMAIN_TAG(_Name)::TrackingData
//----------------------------------------------------------------------------
namespace Domain {
    struct MEMORY_DOMAIN_NAME(Global) {
        static FMemoryTrackingData& TrackingData;
    };
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) \
    namespace Domain { \
        struct MEMORY_DOMAIN_NAME(_Name) { \
            static FMemoryTrackingData& TrackingData; \
        }; \
    }
#else
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) \
    namespace Domain { \
        typedef MEMORY_DOMAIN_TAG(Global) MEMORY_DOMAIN_NAME(_Name); \
    }
#endif
//----------------------------------------------------------------------------
#ifdef COLLAPSE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_COLLAPSABLE_IMPL(_Name, _Parent) \
    namespace Domain { \
        typedef MEMORY_DOMAIN_TAG(_Parent) MEMORY_DOMAIN_NAME(_Name); \
    }
#endif
//----------------------------------------------------------------------------
#include "Core/Memory/MemoryDomain.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef MEMORY_DOMAIN_COLLAPSABLE_IMPL
#undef MEMORY_DOMAIN_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
class TMemoryTracking {
public:
    TMemoryTracking();
    ~TMemoryTracking();
};
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
TMemoryTracking<T, _Domain>::TMemoryTracking() {
    _Domain::TrackingData.Allocate(1, sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
TMemoryTracking<T, _Domain>::~TMemoryTracking() {
    _Domain::TrackingData.Deallocate(1, sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TMemoryView<FMemoryTrackingData *> EachDomainTrackingData();
//----------------------------------------------------------------------------
void ReportDomainTrackingData();
//----------------------------------------------------------------------------
void RegisterAdditionalTrackingData(FMemoryTrackingData *pTrackingData);
void UnregisterAdditionalTrackingData(FMemoryTrackingData *pTrackingData);
//----------------------------------------------------------------------------
void ReportAdditionalTrackingData();
//----------------------------------------------------------------------------
inline void ReportAllTrackingData() {
    ReportDomainTrackingData();
    ReportAdditionalTrackingData();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryDomainStartup {
public:
    static void Start();
    static void Shutdown();

    FMemoryDomainStartup() { Start(); }
    ~FMemoryDomainStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
