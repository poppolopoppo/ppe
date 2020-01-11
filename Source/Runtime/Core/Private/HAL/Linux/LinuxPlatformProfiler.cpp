#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformProfiler.h"

#if USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_LINUX)

#   include "HAL/Linux/VSPerfWrapper.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(int(::PROFILE_GLOBALLEVEL) == int(FLinuxPlatformProfiler::GlobalLevel));
STATIC_ASSERT(int(::PROFILE_PROCESSLEVEL) == int(FLinuxPlatformProfiler::ProcessLevel));
STATIC_ASSERT(int(::PROFILE_THREADLEVEL) == int(FLinuxPlatformProfiler::ThreadLevel));
//----------------------------------------------------------------------------
void FLinuxPlatformProfiler::Start(EProfilerLevel level) {
    FVSPerfWrapper::API.StartProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProfiler::Stop(EProfilerLevel level) {
    FVSPerfWrapper::API.StopProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProfiler::Resume(EProfilerLevel level) {
    FVSPerfWrapper::API.ResumeProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProfiler::Suspend(EProfilerLevel level) {
    FVSPerfWrapper::API.SuspendProfile((::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProfiler::Mark(u32 marker) {
    Assert((long)marker > 0);
    FVSPerfWrapper::API.MarkProfile(checked_cast<long>(marker));
}
//----------------------------------------------------------------------------
void FLinuxPlatformProfiler::MarkAndComment(u32 marker, const char* comment) {
    Assert((long)marker > 0);
    Assert(comment);
    FVSPerfWrapper::API.CommentMarkProfileA(checked_cast<long>(marker), comment);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProfiler::Name(EProfilerLevel level, const char* identifier) {
    Assert(identifier);
    FVSPerfWrapper::API.NameProfileA(identifier, (::PROFILE_CONTROL_LEVEL)level, ::PROFILE_CURRENTID);
}
//----------------------------------------------------------------------------
u32 FLinuxPlatformProfiler::NextMarker() {
    static std::atomic<u32> GCurrentMarker{ 1000 };
    return ++GCurrentMarker;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_PROFILER && defined(PLATFORM_LINUX)