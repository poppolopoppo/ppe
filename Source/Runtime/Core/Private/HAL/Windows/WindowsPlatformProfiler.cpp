#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformProfiler.h"

#if USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_WINDOWS)

#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <atomic> // NextMarker()
#include <vsperf.h>
//#pragma comment(lib, "vsperf.lib") // already done by vsperf.lib

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(int(::PROFILE_GLOBALLEVEL) == int(FWindowsPlatformProfiler::GlobalLevel));
STATIC_ASSERT(int(::PROFILE_PROCESSLEVEL) == int(FWindowsPlatformProfiler::ProcessLevel));
STATIC_ASSERT(int(::PROFILE_THREADLEVEL) == int(FWindowsPlatformProfiler::ThreadLevel));
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Start(EProfilerLevel level) {
    ::StartProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Stop(EProfilerLevel level) {
    ::StopProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Resume(EProfilerLevel level) {
    ::ResumeProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Suspend(EProfilerLevel level) {
    ::SuspendProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Mark(u32 marker) {
    Assert((long)marker > 0);
    ::MarkProfile(checked_cast<long>(marker));
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::MarkAndComment(u32 marker, const char* comment) {
    Assert((long)marker > 0);
    Assert(comment);
    ::CommentMarkProfileA(checked_cast<long>(marker), comment);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Name(EProfilerLevel level, const char* identifier) {
    Assert(identifier);
    ::NameProfileA(identifier, (::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
u32 FWindowsPlatformProfiler::NextMarker() {
    static std::atomic<u32> GCurrentMarker{ 1000 };
    return ++GCurrentMarker;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_WINDOWS)