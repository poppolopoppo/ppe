#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformProfiler.h"

#if USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_WINDOWS)

#   include "HAL/Windows/VSPerfWrapper.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(int(::PROFILE_GLOBALLEVEL) == int(FWindowsPlatformProfiler::GlobalLevel));
STATIC_ASSERT(int(::PROFILE_PROCESSLEVEL) == int(FWindowsPlatformProfiler::ProcessLevel));
STATIC_ASSERT(int(::PROFILE_THREADLEVEL) == int(FWindowsPlatformProfiler::ThreadLevel));
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Start(EProfilerLevel level) {
    FVSPerfWrapper::API.StartProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Stop(EProfilerLevel level) {
    FVSPerfWrapper::API.StopProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Resume(EProfilerLevel level) {
    FVSPerfWrapper::API.ResumeProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Suspend(EProfilerLevel level) {
    FVSPerfWrapper::API.SuspendProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Mark(u32 marker) {
    Assert((long)marker > 0);
    FVSPerfWrapper::API.MarkProfile(checked_cast<long>(marker));
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::MarkAndComment(u32 marker, const char* comment) {
    Assert((long)marker > 0);
    Assert(comment);
    FVSPerfWrapper::API.CommentMarkProfileA(checked_cast<long>(marker), comment);
}
//----------------------------------------------------------------------------
void FWindowsPlatformProfiler::Name(EProfilerLevel level, const char* identifier) {
    Assert(identifier);
    FVSPerfWrapper::API.NameProfileA(identifier, (::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
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