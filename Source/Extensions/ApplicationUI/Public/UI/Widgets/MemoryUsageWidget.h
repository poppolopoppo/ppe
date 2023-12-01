#pragma once

#include "UI/Imgui.h"

#include "Container/Vector.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Memory/MemoryTracking.h"
#include "Memory/PtrRef.h"
#include "Memory/RefPtr.h"
#include "Thread/ThreadSafe.h"
#include "Memory/UniqueView.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryUsageWidget final : ITrackingDataObserver {
public:
    FString Title{ ICON_CI_DEBUG " Memory usage" };

    bool CollapseMemoryDomains{ false };
    bool ExpandMemoryDomains{ false };
    bool HasMemoryDomainObserver{ false };
    bool ShowEmptyMemoryDomains{ false };
    bool WindowVisible{ true };

    FWD_REFPTR(MemoryDomain);
    class FMemoryDomain : public FRefCountable {
    public:
        const TPtrRef<const FMemoryTracking> TrackingData;

        SMemoryDomain Parent;
        SMemoryDomain Children;
        SMemoryDomain Siblings;

        explicit FMemoryDomain(FMemoryTracking& trackingData) NOEXCEPT
        :   TrackingData(trackingData)
        {}
    };
    TThreadSafe<VECTOR(UI, PMemoryDomain), EThreadBarrier::RWLock> MemoryDomains;

    STATIC_CONST_INTEGRAL(int, PlotCapacity, 2048);
    using plotdata_type = INDIRECT_ARRAY(UI, float, PlotCapacity);
    plotdata_type GpuPlot{ 0 };
    plotdata_type UsedPlot{ 0 };
    plotdata_type ReservedPlot{ 0 };
    plotdata_type PooledPlot{ 0 };
    plotdata_type VirtualPlot{ 0 };
    plotdata_type UnaccountedPlot{ 0 };

    plotdata_type PagefileUsage{ 0 };
    plotdata_type WorkingSetSize{ 0 };

    PPE_APPLICATIONUI_API FMemoryUsageWidget();
    PPE_APPLICATIONUI_API ~FMemoryUsageWidget();

    NODISCARD PPE_APPLICATIONUI_API bool Show();

    PPE_APPLICATIONUI_API virtual void OnRegisterTrackingData(FMemoryTracking* pTrackingData) override final;
    PPE_APPLICATIONUI_API virtual void OnUnregisterTrackingData(FMemoryTracking* pTrackingData) override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
