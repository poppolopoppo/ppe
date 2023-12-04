// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Time/Timepoint.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformTime.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTimepoint FTimepoint::Now() {
    return FTimepoint(FPlatformTime::Cycles());
}
//----------------------------------------------------------------------------
FTimepoint::value_type FTimepoint::Ticks(const FTimespan& duration) {
    Assert(duration.Value() >= 0);

    return FPlatformTime::ToCycles(duration.Value());
}
//----------------------------------------------------------------------------
FTimespan FTimepoint::Duration(const FTimepoint& start, const FTimepoint& stop) {
    Assert(start.Value() <= stop.Value());

    const FTimepoint cycles = (stop.Value() - start.Value());

    return Units::Time::FSeconds(FPlatformTime::ToSeconds(cycles.Value()));
}
//----------------------------------------------------------------------------
FTimespan FTimepoint::SignedDuration(const FTimepoint& start, const FTimepoint& stop) {
    if (Likely(start <= stop))
        return Duration(start, stop);

    const FTimespan duration = Duration(stop, start);
    return -duration.Value();
}
//----------------------------------------------------------------------------
FTimestamp FTimepoint::Timestamp() const {
    // add elapsed duration since process start and add it to process time stamp
    const FCurrentProcess& process = FCurrentProcess::Get();
    const FTimespan elapsedSinceProcessStart = SignedDuration(process.StartTicks(), *this);
    return (process.StartDate() + elapsedSinceProcessStart);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
