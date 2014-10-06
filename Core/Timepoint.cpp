#include "stdafx.h"

#include "Timepoint.h"

#ifdef OS_WINDOWS
#   include <Windows.h>
#else
#   error "no support !"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
    // Retrieves QPC frequency one time per run

    struct QueryPerformanceCounterContext {
        QueryPerformanceCounterContext();
        LARGE_INTEGER Frequency;

        STATIC_ASSERT(sizeof(LARGE_INTEGER) == sizeof(Timepoint));
    };

    QueryPerformanceCounterContext::QueryPerformanceCounterContext() {
        QueryPerformanceFrequency(&Frequency);
    }

    static const QueryPerformanceCounterContext gQPC_Context;
} //!namespace
//----------------------------------------------------------------------------
Timepoint Timepoint::Now() {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return *reinterpret_cast<const value_type *>(&now);
}
//----------------------------------------------------------------------------
Timepoint::value_type Timepoint::Ticks(const Timespan& duration) {
    const double d = (duration.Value() * gQPC_Context.Frequency.QuadPart)/1000000;
    return static_cast<Timepoint::value_type>(d);
}
//----------------------------------------------------------------------------
Timespan Timepoint::Duration(const Timepoint& start, const Timepoint& stop) {
    LARGE_INTEGER elapsedMicroS;
    elapsedMicroS.QuadPart = reinterpret_cast<const LARGE_INTEGER *>(&stop)->QuadPart -
                             reinterpret_cast<const LARGE_INTEGER *>(&start)->QuadPart;

    elapsedMicroS.QuadPart *= 1000000;
    elapsedMicroS.QuadPart /= gQPC_Context.Frequency.QuadPart;

    return Units::Time::Microseconds(static_cast<double>(elapsedMicroS.QuadPart));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
