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
#   include "IO/FormatHelpers.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Domain {
    FMemoryTracking& MEMORY_DOMAIN_NAME(Global)::TrackingData = FMemoryTracking::Global();
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
#define MEMORY_DOMAIN_IMPL(_Name, _Parent) \
    namespace Domain { \
        namespace { \
            static FMemoryTrackingData CONCAT(GTrackingData, MEMORY_DOMAIN_NAME(_Name)) { \
                STRINGIZE(_Name), &MEMORY_DOMAIN_TRACKING_DATA(_Parent) \
            }; \
        }\
        FMemoryTrackingData& MEMORY_DOMAIN_NAME(_Name)::TrackingData = \
            CONCAT(GTrackingData, MEMORY_DOMAIN_NAME(_Name)); \
    }

#ifdef COLLAPSE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_COLLAPSABLE_IMPL(_Name, _Parent)
#endif

#include "MemoryDomain.Definitions-inl.h"

#undef MEMORY_DOMAIN_COLLAPSABLE_IMPL
#undef MEMORY_DOMAIN_IMPL
//----------------------------------------------------------------------------
#endif //!USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
namespace {
//----------------------------------------------------------------------------
#define MEMORY_DOMAIN_IMPL(_Name, _Parent) &MEMORY_DOMAIN_TRACKING_DATA(_Name) COMMA

#ifdef COLLAPSE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_COLLAPSABLE_IMPL(_Name, _Parent)
#endif

static FMemoryTracking *GAllMemoryDomainTrackingData[] = {
#include "MemoryDomain.Definitions-inl.h"
};

#undef MEMORY_DOMAIN_COLLAPSABLE_IMPL
#undef MEMORY_DOMAIN_IMPL
//----------------------------------------------------------------------------
} //!namespace
#endif //!USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
struct FAdditionalTrackingData {
    std::mutex Barrier;
    STATIC_CONST_INTEGRAL(size_t, Capacity, 2048);
    VECTORINSITU(Internal, FMemoryTracking*, Capacity) Datas;
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
TMemoryView<FMemoryTracking *> EachDomainTrackingData() {
#ifdef USE_MEMORY_DOMAINS
    return MakeView(GAllMemoryDomainTrackingData);
#else
    return TMemoryView<FMemoryTracking *>();
#endif
}
//----------------------------------------------------------------------------
void ReportDomainTrackingData() {
#if defined(USE_MEMORY_DOMAINS) && defined(USE_DEBUG_LOGGER)
    const FMemoryTracking **ptr = (const FMemoryTracking **)&GAllMemoryDomainTrackingData[0];
    const TMemoryView<const FMemoryTracking *> datas(ptr, lengthof(GAllMemoryDomainTrackingData));
    FLoggerStream log(ELogCategory::Debug);
    ReportTrackingDatas(log, L"Memory Domains", datas);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterAdditionalTrackingData(FMemoryTracking *pTrackingData) {
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
void UnregisterAdditionalTrackingData(FMemoryTracking *pTrackingData) {
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
void ReportAllocationHistogram() {
#if defined(USE_MEMORY_DOMAINS) && defined(USE_DEBUG_LOGGER)
    TMemoryView<const size_t> classes, allocations, totalBytes;
    if (not FetchMemoryAllocationHistogram(&classes, &allocations, &totalBytes))
        return;

    Assert(classes.size() == allocations.size());

    const auto distribution = [](size_t sz) -> float {
        return std::log((float)sz + 1.0f) - std::log(1.0f);
    };

    float totalCount = 0;
    float allocationScale = 0;
    for (size_t count : allocations) {
        totalCount += count;
        allocationScale = Max(allocationScale, distribution(count));
    }

    LOG(Info, L"[Malloc] report allocations size histogram");

    static size_t GPrevAllocations[60/* chhh */] = { 0 };
    AssertRelease(lengthof(GPrevAllocations) == classes.size());

    constexpr float width = 80;
    forrange(i, 0, classes.size()) {
        if (0 == classes[i]) continue;

        if (0 == allocations[i])
            LOG(Info, L" #{0:#2} | {1:9} | {2:9} | {3:5f2}% |",
                i,
                Fmt::FSizeInBytes{ classes[i] },
                Fmt::FSizeInBytes{ totalBytes[i] },
                100 * float(allocations[i]) / totalCount );
        else
            LOG(Info, L" #{0:#2} | {1:9} | {2:9} | {3:5f2}% |{4}> {5} +{6}",
                i,
                Fmt::FSizeInBytes{ classes[i] },
                Fmt::FSizeInBytes{ totalBytes[i] },
                100 * float(allocations[i]) / totalCount,
                Fmt::Repeat(L'=', size_t(Min(width, std::round(width * distribution(allocations[i]) / allocationScale)))),
                Fmt::FCountOfElements{ allocations[i] },
                allocations[i] - GPrevAllocations[i] );

        GPrevAllocations[i] = allocations[i];
    }
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
