#include "stdafx.h"

#include "MemoryDomain.h"

#include "MemoryTracking.h"
#include "MemoryView.h"

#ifdef USE_MEMORY_DOMAINS
#   include <algorithm>
#   include <iostream>
#   include <mutex>
#   include "Container/Vector.h"
#   include "Diagnostic/CrtDebug.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Domain {
    MemoryTrackingData& MEMORY_DOMAIN_NAME(Global)::TrackingData = MemoryTrackingData::Global();
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) namespace Domain { \
    namespace { static MemoryTrackingData CONCAT(gTrackingData, MEMORY_DOMAIN_NAME(_Name)) { STRINGIZE(_Name), &MEMORY_DOMAIN_TRACKING_DATA(_Parent) }; }\
    MemoryTrackingData& MEMORY_DOMAIN_NAME(_Name)::TrackingData = CONCAT(gTrackingData, MEMORY_DOMAIN_NAME(_Name)); \
    }
#else
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent)
#endif
//----------------------------------------------------------------------------
#include "MemoryDomain.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef MEMORY_DOMAIN_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static MemoryTrackingData *gAllMemoryDomainTrackingData[] = {
//----------------------------------------------------------------------------
    &MEMORY_DOMAIN_TRACKING_DATA(Global)
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) COMMA &MEMORY_DOMAIN_TRACKING_DATA(_Name)
#else
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent)
#endif
//----------------------------------------------------------------------------
#include "MemoryDomain.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef MEMORY_DOMAIN_IMPL
//----------------------------------------------------------------------------
}; //!gAllMemoryDomainTrackingData[]
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
struct AdditionalTrackingData {
    std::mutex Barrier;
    std::vector<MemoryTrackingData *> Datas;
};
static AdditionalTrackingData *gAllAdditionalTrackingData = nullptr;
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryView<MemoryTrackingData *> EachDomainTrackingData() {
#ifdef USE_MEMORY_DOMAINS
    return MakeView(gAllMemoryDomainTrackingData);
#else
    return MemoryView<MemoryTrackingData *>();
#endif
}
//----------------------------------------------------------------------------
void ReportDomainTrackingData() {
#ifdef USE_MEMORY_DOMAINS
    const MemoryTrackingData **ptr = (const MemoryTrackingData **)&gAllMemoryDomainTrackingData[0];
    const MemoryView<const MemoryTrackingData *> datas(ptr, lengthof(gAllMemoryDomainTrackingData));
    ReportTrackingDatas(std::cout, "Memory Domains", datas);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterAdditionalTrackingData(MemoryTrackingData *pTrackingData) {
#ifdef USE_MEMORY_DOMAINS
    Assert(pTrackingData);
    AssertRelease(gAllAdditionalTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    std::unique_lock<std::mutex> scopeLock(gAllAdditionalTrackingData->Barrier);
    Add_AssertUnique(gAllAdditionalTrackingData->Datas, pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void UnregisterAdditionalTrackingData(MemoryTrackingData *pTrackingData) {
#ifdef USE_MEMORY_DOMAINS
    Assert(pTrackingData);
    AssertRelease(gAllAdditionalTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    std::unique_lock<std::mutex> scopeLock(gAllAdditionalTrackingData->Barrier);
    Remove_AssertExists(gAllAdditionalTrackingData->Datas, pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void ReportAdditionalTrackingData() {
#ifdef USE_MEMORY_DOMAINS
    AssertRelease(gAllAdditionalTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    std::unique_lock<std::mutex> scopeLock(gAllAdditionalTrackingData->Barrier);
    const MemoryTrackingData **ptr = (const MemoryTrackingData **)&gAllAdditionalTrackingData->Datas[0];
    const MemoryView<const MemoryTrackingData *> datas(ptr, gAllAdditionalTrackingData->Datas.size());
    ReportTrackingDatas(std::cout, "Additional", datas);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void MemoryDomainStartup::Start() {
#ifdef USE_MEMORY_DOMAINS
    Assert(nullptr == gAllAdditionalTrackingData);
    gAllAdditionalTrackingData = new AdditionalTrackingData();
#endif
}
//----------------------------------------------------------------------------
void MemoryDomainStartup::Shutdown() {
#ifdef USE_MEMORY_DOMAINS
    Assert(nullptr != gAllAdditionalTrackingData);
    checked_delete(gAllAdditionalTrackingData);
    gAllAdditionalTrackingData = nullptr;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
