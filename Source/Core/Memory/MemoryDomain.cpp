#include "stdafx.h"

#include "MemoryDomain.h"

#include "MemoryTracking.h"
#include "MemoryView.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/Stream.h"
#include "IO/String.h"

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
MemoryView<MemoryTrackingData*> EachDomainTrackingData() {
    return MakeView(gAllMemoryDomainTrackingData);
}
//----------------------------------------------------------------------------
static void TrackingDataAbsoluteName_(Core::OCStrStream&& oss, const Core::MemoryTrackingData& trackingData) {
    if (trackingData.Parent()) {
        TrackingDataAbsoluteName_(std::move(oss), *trackingData.Parent());
        oss << "::";
    }
    oss << trackingData.Name();
}
//----------------------------------------------------------------------------
void ReportDomainTrackingData() {
#ifdef USE_MEMORY_DOMAINS
    std::cout << Repeat<90>("-") << std::endl;

    Format(std::cout, " {0:-34}|{1:7} {2:7} |{3:6} {4:6} |{5:10} {6:10}\n",
        "Tracking Data Name",
        "Block", "Max",
        "Alloc", "Max",
        "Total", "Max"
        );

    std::cout << Repeat<90>("-") << std::endl;

    char absoluteName[1024];
    for (const auto trackingData : EachDomainTrackingData()) {
        TrackingDataAbsoluteName_(OCStrStream(absoluteName), *trackingData);
        Format(std::cout, " {0:-34}|{1:7} {2:7} |{3:6} {4:6} |{5:10} {6:10}\n",
            absoluteName,
            trackingData->BlockCount(),
            trackingData->MaxBlockCount(),
            trackingData->AllocationCount(),
            trackingData->MaxAllocationCount(),
            trackingData->TotalSizeInBytes(),
            trackingData->MaxTotalSizeInBytes()
            );
    }

    std::cout << Repeat<90>("-") << std::endl;
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
