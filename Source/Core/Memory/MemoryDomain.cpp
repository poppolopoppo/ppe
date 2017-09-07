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
    FMemoryTrackingData& MEMORY_DOMAIN_NAME(Global)::TrackingData = FMemoryTrackingData::Global();
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) \
    namespace Domain { \
        namespace { \
            static FMemoryTrackingData CONCAT(GTrackingData, MEMORY_DOMAIN_NAME(_Name)) { \
                STRINGIZE(_Name), &MEMORY_DOMAIN_TRACKING_DATA(_Parent) \
            }; \
        }\
        FMemoryTrackingData& MEMORY_DOMAIN_NAME(_Name)::TrackingData = \
            CONCAT(GTrackingData, MEMORY_DOMAIN_NAME(_Name)); \
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
static FMemoryTrackingData *GAllMemoryDomainTrackingData[] = {
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) &MEMORY_DOMAIN_TRACKING_DATA(_Name) COMMA
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
}; //!namespace
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
struct FAdditionalTrackingData {
    std::mutex Barrier;
    STATIC_CONST_INTEGRAL(size_t, Capacity, 2048);
    VECTORINSITU(Internal, FMemoryTrackingData*, Capacity) Datas;
    FAdditionalTrackingData() {
        // don't want to allocate on other threads :
        Datas.reserve_AssumeEmpty(Capacity);
    }
};
static FAdditionalTrackingData* GAllAdditionalTrackingData = nullptr;
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TMemoryView<FMemoryTrackingData *> EachDomainTrackingData() {
#ifdef USE_MEMORY_DOMAINS
    return MakeView(GAllMemoryDomainTrackingData);
#else
    return TMemoryView<FMemoryTrackingData *>();
#endif
}
//----------------------------------------------------------------------------
void ReportDomainTrackingData() {
#if defined(USE_MEMORY_DOMAINS) && defined(USE_DEBUG_LOGGER)
    const FMemoryTrackingData **ptr = (const FMemoryTrackingData **)&GAllMemoryDomainTrackingData[0];
    const TMemoryView<const FMemoryTrackingData *> datas(ptr, lengthof(GAllMemoryDomainTrackingData));
    FLoggerStream log(ELogCategory::Debug);
    ReportTrackingDatas(log, L"Memory Domains", datas);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterAdditionalTrackingData(FMemoryTrackingData *pTrackingData) {
#ifdef USE_MEMORY_DOMAINS
    Assert(pTrackingData);
    AssertRelease(GAllAdditionalTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    std::unique_lock<std::mutex> scopeLock(GAllAdditionalTrackingData->Barrier);
    Assert(not Contains(GAllAdditionalTrackingData->Datas, pTrackingData));
    GAllAdditionalTrackingData->Datas.push_back_AssumeNoGrow(pTrackingData);
#else
    UNUSED(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void UnregisterAdditionalTrackingData(FMemoryTrackingData *pTrackingData) {
#ifdef USE_MEMORY_DOMAINS
    Assert(pTrackingData);
    AssertRelease(GAllAdditionalTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    std::unique_lock<std::mutex> scopeLock(GAllAdditionalTrackingData->Barrier);
    Remove_AssertExists(GAllAdditionalTrackingData->Datas, pTrackingData);
#else
    UNUSED(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void ReportAdditionalTrackingData() {
#if defined(USE_MEMORY_DOMAINS) && defined(USE_DEBUG_LOGGER)
    AssertRelease(GAllAdditionalTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    FLoggerStream log(ELogCategory::Debug);
    std::unique_lock<std::mutex> scopeLock(GAllAdditionalTrackingData->Barrier);
    ReportTrackingDatas(log, L"Additional", GAllAdditionalTrackingData->Datas.MakeConstView());
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FMemoryDomainStartup::Start() {
#ifdef USE_MEMORY_DOMAINS
    Assert(nullptr == GAllAdditionalTrackingData);
    GAllAdditionalTrackingData = new FAdditionalTrackingData();
#endif
}
//----------------------------------------------------------------------------
void FMemoryDomainStartup::Shutdown() {
#ifdef USE_MEMORY_DOMAINS
    Assert(nullptr != GAllAdditionalTrackingData);
    checked_delete_ref(GAllAdditionalTrackingData);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
