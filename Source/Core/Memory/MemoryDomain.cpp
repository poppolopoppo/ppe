#include "stdafx.h"

#include "MemoryDomain.h"

#include "MemoryTracking.h"
#include "MemoryView.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/Stream.h"
#include "IO/String.h"

#include "Thread/ThreadContext.h"

#include <iostream>

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
static MemoryTrackingData* gAllMemoryDomainTrackingData[] = {
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
static volatile size_t gAllAdditionalTrackingDataCount = 0;
static volatile MemoryTrackingData* gAllAdditionalTrackingData[1024] = {nullptr};
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void TrackingDataAbsoluteName_(Core::OCStrStream&& oss, const Core::MemoryTrackingData& trackingData) {
    if (trackingData.Parent()) {
        TrackingDataAbsoluteName_(std::move(oss), *trackingData.Parent());
        oss << "::";
    }
    oss << trackingData.Name();
}
//----------------------------------------------------------------------------
static void ReportTrackingData_(const char *name, const MemoryView<MemoryTrackingData *>& trackingDatas) {
    const size_t width = 139;
    const char *fmt = " {0:-73}|{1:10} {2:10} |{3:7} {4:7} |{5:11} {6:11}\n";

    std::cout   << Repeat<width>("-") << std::endl
                << "    " << name << std::endl
                << Repeat<width>("-") << std::endl;

    Format(std::cout, fmt,
        "Tracking Data Name",
        "Block", "Max",
        "Alloc", "Max",
        "Total", "Max" );

    std::cout << Repeat<width>("-") << std::endl;

    char absoluteName[1024];
    for (const auto trackingData : trackingDatas) {
        TrackingDataAbsoluteName_(OCStrStream(absoluteName), *trackingData);
        Format(std::cout, fmt,
            absoluteName,
            trackingData->BlockCount(),
            trackingData->MaxBlockCount(),
            trackingData->AllocationCount(),
            trackingData->MaxAllocationCount(),
            trackingData->TotalSizeInBytes(),
            trackingData->MaxTotalSizeInBytes() );
    }

    std::cout << Repeat<width>("-") << std::endl;
}
//----------------------------------------------------------------------------
static bool LessTrackingData_(const MemoryTrackingData& lhs, const MemoryTrackingData& rhs) {
    Assert(lhs.Name());
    Assert(rhs.Name());
    return (lhs.Name() != rhs.Name()) && CompareI(lhs.Name(), rhs.Name()) < 0;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
#endif //!USE_MEMORY_DOMAINS
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
    ReportTrackingData_("Memory Domains", EachDomainTrackingData());
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterAdditionalTrackingData(MemoryTrackingData& trackingData) {
#ifdef USE_MEMORY_DOMAINS
    AssertRelease(gAllAdditionalTrackingDataCount < lengthof(gAllAdditionalTrackingData));
    gAllAdditionalTrackingData[gAllAdditionalTrackingDataCount++] = &trackingData;
#endif
}
//----------------------------------------------------------------------------
MemoryView<MemoryTrackingData *> EachAdditionalTrackingData() {
#ifdef USE_MEMORY_DOMAINS
    Assert(IsInMainThread());
    return MemoryView<MemoryTrackingData *>((MemoryTrackingData **)&gAllAdditionalTrackingData[0], size_t(gAllAdditionalTrackingDataCount));
#else
    return MemoryView<MemoryTrackingData *>();
#endif
}
//----------------------------------------------------------------------------
void ReportAdditionalTrackingData() {
#ifdef USE_MEMORY_DOMAINS
    Assert(IsInMainThread());
    const MemoryView<MemoryTrackingData *> datas = EachAdditionalTrackingData();
    std::sort(datas.begin(), datas.end(), [](MemoryTrackingData *lhs, MemoryTrackingData *rhs) {
        MemoryTrackingData *lhsp = lhs->Parent();
        MemoryTrackingData *rhsp = rhs->Parent();
        if (lhsp && rhsp)
            return (LessTrackingData_(*lhsp, *rhsp) || (lhsp->Name() == rhsp->Name() && LessTrackingData_(*lhs, *rhs)));
        else if (lhsp)
            return LessTrackingData_(*lhsp, *rhs);
        else if (rhsp)
            return lhs->Name() == rhsp->Name() || LessTrackingData_(*lhs, *rhsp);
        else
            return LessTrackingData_(*lhs, *rhs);
    });
    ReportTrackingData_("Additional", EachAdditionalTrackingData());
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
