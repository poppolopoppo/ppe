#include "stdafx.h"

#include "MemoryDomain.h"

#include "MemoryTracking.h"
#include "MemoryView.h"

#include "Diagnostic/Logger.h"
#include "Meta/OneTimeInitialize.h"

#ifdef USE_MEMORY_DOMAINS
#   include "Container/Vector.h"
#   include "Diagnostic/CrtDebug.h"
#   include "IO/FormatHelpers.h"
#   include "IO/String.h"
#   include "IO/StringBuilder.h"
#   include "Thread/AtomicSpinLock.h"

#   include <algorithm>
#endif

namespace Core {
LOG_CATEGORY(CORE_API, MemoryTracking)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Domain {
    FMemoryTracking& MEMORY_DOMAIN_NAME(Global)::TrackingData() {
        return FMemoryTracking::Global();
    }
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
#define MEMORY_DOMAIN_IMPL(_Name, _Parent) \
    namespace Domain { \
        FMemoryTracking& MEMORY_DOMAIN_NAME(_Name)::TrackingData() { \
            ONE_TIME_INITIALIZE(FMemoryTracking, GTrackingData, STRINGIZE(_Name), &MEMORY_DOMAIN_TRACKING_DATA(_Parent)); \
            return GTrackingData; \
        } \
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
    FAtomicSpinLock Barrier;
    VECTOR(Internal, FMemoryTracking*) Datas;
    FAdditionalTrackingData() {
        // don't want to allocate on other threads :
        Datas.reserve_AssumeEmpty(1024);
    }
};
static FAdditionalTrackingData& AllAdditionalTrackingData_() {
    static FAdditionalTrackingData GInstance;
    return GInstance;
}
#endif //!USE_MEMORY_DOMAINS
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

    FWStringBuilder oss;
    ReportTrackingDatas(oss, L"Memory Domains", datas);

    LOG(MemoryTracking, Info, oss.ToString());
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterAdditionalTrackingData(FMemoryTracking *pTrackingData) {
#ifdef USE_MEMORY_DOMAINS
    Assert(pTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    FAdditionalTrackingData& Additional = AllAdditionalTrackingData_();
    const FAtomicSpinLock::FScope scopeLock(Additional.Barrier);
    Assert(not Contains(Additional.Datas, pTrackingData));
    Additional.Datas.push_back_AssumeNoGrow(pTrackingData);
#else
    UNUSED(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void UnregisterAdditionalTrackingData(FMemoryTracking *pTrackingData) {
#ifdef USE_MEMORY_DOMAINS
    Assert(pTrackingData);
    SKIP_MEMORY_LEAKS_IN_SCOPE();
    FAdditionalTrackingData& Additional = AllAdditionalTrackingData_();
    const FAtomicSpinLock::FScope scopeLock(Additional.Barrier);
    Remove_AssertExists(Additional.Datas, pTrackingData);
#else
    UNUSED(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void ReportAdditionalTrackingData() {
#if defined(USE_MEMORY_DOMAINS) && defined(USE_DEBUG_LOGGER)
    SKIP_MEMORY_LEAKS_IN_SCOPE();

    FWStringBuilder oss;
    FAdditionalTrackingData& Additional = AllAdditionalTrackingData_();

    const FAtomicSpinLock::FScope scopeLock(Additional.Barrier);
    ReportTrackingDatas(oss, L"Additional", Additional.Datas.MakeConstView());

    LOG(MemoryTracking, Info, oss.ToString());
#endif
}
//----------------------------------------------------------------------------
void ReportAllocationHistogram() {
#if defined(USE_MEMORY_DOMAINS) && defined(USE_DEBUG_LOGGER)
    TMemoryView<const size_t> classes, allocations, totalBytes;
    if (not FetchMemoryAllocationHistogram(&classes, &allocations, &totalBytes))
        return;

    Assert(classes.size() == allocations.size());

    size_t totalCount = 0;
    size_t maxCount = 0;
    for (size_t count : allocations) {
        totalCount += count;
        maxCount = Max(maxCount, count);
    }

    const auto distribution = [](size_t sz) -> float {
        return (std::log((float)sz + 1.f) - std::log(1.f));
    };

    const float distributionScale = distribution(maxCount);

    FWStringBuilder oss;
    oss << FTextFormat::Float(FTextFormat::FixedFloat, 2);
    oss << L"report allocations size histogram" << Eol;

    static size_t GPrevAllocations[60/* chhh */] = { 0 };
    AssertRelease(lengthof(GPrevAllocations) == classes.size());

    constexpr float width = 80;
    forrange(i, 0, classes.size()) {
        if (0 == classes[i]) continue;

        if (0 == allocations[i])
            Format(oss, L" #{0:#2} | {1:9} | {2:9} | {3:5}% |",
                i,
                Fmt::FSizeInBytes{ classes[i] },
                Fmt::FSizeInBytes{ totalBytes[i] },
                0.f );
        else
            Format(oss, L" #{0:#2} | {1:9} | {2:9} | {3:5}% |{4}> {5} +{6}",
                i,
                Fmt::FSizeInBytes{ classes[i] },
                Fmt::FSizeInBytes{ totalBytes[i] },
                100 * float(allocations[i]) / totalCount,
                Fmt::Repeat(L'=', size_t(std::round(Min(width, width * distribution(allocations[i]) / distributionScale)))),
                Fmt::FCountOfElements{ allocations[i] },
                allocations[i] - GPrevAllocations[i] );

        oss << Eol;

        GPrevAllocations[i] = allocations[i];
    }

    LOG(MemoryTracking, Info, oss.ToString());
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
