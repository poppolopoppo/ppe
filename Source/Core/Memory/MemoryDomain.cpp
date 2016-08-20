#include "stdafx.h"

#include "MemoryDomain.h"

#include "MemoryTracking.h"
#include "MemoryView.h"

#include "Diagnostic/Logger.h"

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
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) \
    namespace Domain { \
        namespace { \
            static MemoryTrackingData CONCAT(gTrackingData, MEMORY_DOMAIN_NAME(_Name)) { \
                STRINGIZE(_Name), &MEMORY_DOMAIN_TRACKING_DATA(_Parent) \
            }; \
        }\
        MemoryTrackingData& MEMORY_DOMAIN_NAME(_Name)::TrackingData = \
            CONCAT(gTrackingData, MEMORY_DOMAIN_NAME(_Name)); \
    }
#else
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent)
#endif
//----------------------------------------------------------------------------
#ifdef COLLAPSE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_COLLAPSABLE_IMPL(_Name, _Parent)
#endif
//----------------------------------------------------------------------------
#include "MemoryDomain.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef MEMORY_DOMAIN_COLLAPSABLE_IMPL
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
#ifdef COLLAPSE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_COLLAPSABLE_IMPL(_Name, _Parent)
#endif
//----------------------------------------------------------------------------
#include "MemoryDomain.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef MEMORY_DOMAIN_COLLAPSABLE_IMPL
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
    Vector<MemoryTrackingData *, std::allocator<MemoryTrackingData *>> Datas;
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
#if defined(USE_MEMORY_DOMAINS) && defined(USE_DEBUG_LOGGER)
    const MemoryTrackingData **ptr = (const MemoryTrackingData **)&gAllMemoryDomainTrackingData[0];
    const MemoryView<const MemoryTrackingData *> datas(ptr, lengthof(gAllMemoryDomainTrackingData));
    LoggerStream log(LogCategory::Debug);
    ReportTrackingDatas(log, L"Memory Domains", datas);
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
#else
    UNUSED(pTrackingData);
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
#else
    UNUSED(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void ReportAdditionalTrackingData() {
#if defined(USE_MEMORY_DOMAINS) && defined(USE_DEBUG_LOGGER)
    AssertRelease(gAllAdditionalTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    std::unique_lock<std::mutex> scopeLock(gAllAdditionalTrackingData->Barrier);
    const MemoryTrackingData **ptr = (const MemoryTrackingData **)&gAllAdditionalTrackingData->Datas[0];
    const MemoryView<const MemoryTrackingData *> datas(ptr, gAllAdditionalTrackingData->Datas.size());
    LoggerStream log(LogCategory::Debug);
    ReportTrackingDatas(log, L"Additional", datas);
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
