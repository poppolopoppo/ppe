#include "stdafx.h"

#include "Timepoint.h"

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

    return FPlatformTime::ToTicks(duration.Value());
}
//----------------------------------------------------------------------------
FTimespan FTimepoint::Duration(const FTimepoint& start, const FTimepoint& stop) {
    Assert(start.Value() <= stop.Value());

    const FTimepoint cycles = (stop.Value() - start.Value());

    return (FPlatformTime::ToSeconds(cycles.Value()) * 1000.0);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
