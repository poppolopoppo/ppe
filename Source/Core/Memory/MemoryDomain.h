#pragma once

#include "Core/Core.h"

#ifndef FINAL_RELEASE
#   define USE_MEMORY_DOMAINS
#endif

namespace Core {
template <typename T>
class MemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MemoryTrackingData;
//----------------------------------------------------------------------------
#define MEMORY_DOMAIN_NAME(_Name) CONCAT(_, _Name)
#define MEMORY_DOMAIN_TAG(_Name) Core::Domain::MEMORY_DOMAIN_NAME(_Name)
#define MEMORY_DOMAIN_TRACKING_DATA(_Name) MEMORY_DOMAIN_TAG(_Name)::TrackingData
//----------------------------------------------------------------------------
namespace Domain {
    struct MEMORY_DOMAIN_NAME(Global) {
        static MemoryTrackingData& TrackingData;
    };
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) namespace Domain { \
    struct MEMORY_DOMAIN_NAME(_Name) { \
        static MemoryTrackingData& TrackingData; \
        }; }
#else
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) namespace Domain { \
    typedef MEMORY_DOMAIN_TAG(Global) MEMORY_DOMAIN_NAME(_Name); }
#endif
//----------------------------------------------------------------------------
#include "Core/Memory/MemoryDomain.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef MEMORY_DOMAIN_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
class MemoryTracking {
public:
    MemoryTracking();
    ~MemoryTracking();
};
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
MemoryTracking<T, _Domain>::MemoryTracking() {
    _Domain::TrackingData.Allocate(1, sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
MemoryTracking<T, _Domain>::~MemoryTracking() {
    _Domain::TrackingData.Deallocate(1, sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryView<MemoryTrackingData *> EachDomainTrackingData();
//----------------------------------------------------------------------------
void ReportDomainTrackingData();
//----------------------------------------------------------------------------
void RegisterAdditionalTrackingData(MemoryTrackingData *pTrackingData);
void UnregisterAdditionalTrackingData(MemoryTrackingData *pTrackingData);
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
class MemoryDomainStartup {
public:
    static void Start();
    static void Shutdown();

    MemoryDomainStartup() { Start(); }
    ~MemoryDomainStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
