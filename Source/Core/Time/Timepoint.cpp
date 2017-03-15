#include "stdafx.h"

#include "Timepoint.h"

#ifdef PLATFORM_WINDOWS
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

    struct FQueryPerformanceCounterContext {
        FQueryPerformanceCounterContext();
        LARGE_INTEGER Frequency;
        STATIC_ASSERT(sizeof(LARGE_INTEGER) == sizeof(FTimepoint));
    };

    FQueryPerformanceCounterContext::FQueryPerformanceCounterContext() {
        QueryPerformanceFrequency(&Frequency);
    }

    static const FQueryPerformanceCounterContext gQPC_Context;
} //!namespace
//----------------------------------------------------------------------------
FTimepoint FTimepoint::Now() {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return *reinterpret_cast<const value_type *>(&now);
}
//----------------------------------------------------------------------------
FTimepoint::value_type FTimepoint::Ticks(const FTimespan& duration) {
    const double d = (duration.Value() * gQPC_Context.Frequency.QuadPart)/1000000;
    return static_cast<FTimepoint::value_type>(d);
}
//----------------------------------------------------------------------------
FTimespan FTimepoint::Duration(const FTimepoint& start, const FTimepoint& stop) {
    LARGE_INTEGER elapsedMicroS;
    elapsedMicroS.QuadPart = reinterpret_cast<const LARGE_INTEGER *>(&stop)->QuadPart -
                             reinterpret_cast<const LARGE_INTEGER *>(&start)->QuadPart;

    elapsedMicroS.QuadPart *= 1000000;
    elapsedMicroS.QuadPart /= gQPC_Context.Frequency.QuadPart;

    return FMicroseconds(static_cast<double>(elapsedMicroS.QuadPart));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
