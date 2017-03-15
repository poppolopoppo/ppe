#include "stdafx.h"

#include "Profiling.h"

#ifdef WITH_CORE_PROFILING

#ifdef PLATFORM_WINDOWS
#   include <VSPerf.h>
#else
#   error "unsuppored platform"
#endif

#include "Memory/AlignedStorage.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(::PROFILE_GLOBALLEVEL   == FProfiler::GlobalLevel);
STATIC_ASSERT(::PROFILE_PROCESSLEVEL  == FProfiler::ProcessLevel);
STATIC_ASSERT(::PROFILE_THREADLEVEL   == FProfiler::ThreadLevel);
//----------------------------------------------------------------------------
STATIC_ASSERT(::PROFILE_CURRENTID     == FProfiler::CurrentID);
//----------------------------------------------------------------------------
FProfiler::FProfiler(ELevel level, u32 id /* = CurrentID */) : _level(level), _id(id) {}
//----------------------------------------------------------------------------
FProfiler::~FProfiler() {}
//----------------------------------------------------------------------------
void FProfiler::Start() const {
    ::StartProfile(::PROFILE_CONTROL_LEVEL(_level), _id);
}
//----------------------------------------------------------------------------
void FProfiler::Stop() const {
    ::StopProfile(::PROFILE_CONTROL_LEVEL(_level), _id);
}
//----------------------------------------------------------------------------
void FProfiler::Resume() const {
    ::ResumeProfile(::PROFILE_CONTROL_LEVEL(_level), _id);
}
//----------------------------------------------------------------------------
void FProfiler::Suspend() const {
    ::SuspendProfile(::PROFILE_CONTROL_LEVEL(_level), _id);
}
//----------------------------------------------------------------------------
void FProfiler::Mark(u32 data) const {
    ::MarkProfile(data);
}
//----------------------------------------------------------------------------
void FProfiler::MarkAndComment(u32 data, const char* cstr) const {
    ::CommentMarkProfileA(data, cstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static POD_STORAGE(FProfiler) _gProfilerPOD_Global;
static FProfiler* _gProfilerPtr_Global = nullptr;
static POD_STORAGE(FProfiler) _gProfilerPOD_Process;
static FProfiler* _gProfilerPtr_Process = nullptr;
static POD_STORAGE(FProfiler) _gProfilerPOD_Thread;
static FProfiler* _gProfilerPtr_Thread = nullptr;
}
//----------------------------------------------------------------------------
void FProfiler::Start() {
    Assert(nullptr == _gProfilerPtr_Global);
    Assert(nullptr == _gProfilerPtr_Process);
    Assert(nullptr == _gProfilerPtr_Thread);

    _gProfilerPtr_Global = new ((void*)&_gProfilerPOD_Global) FProfiler(FProfiler::GlobalLevel);
    _gProfilerPtr_Process = new ((void*)&_gProfilerPOD_Process) FProfiler(FProfiler::ProcessLevel);
    _gProfilerPtr_Thread = new ((void*)&_gProfilerPOD_Thread) FProfiler(FProfiler::ThreadLevel);

    _gProfilerPtr_Global->Stop(); // profiler is started by default
}
//----------------------------------------------------------------------------
void FProfiler::Shutdown() {
    Assert(nullptr != _gProfilerPtr_Global);
    Assert(nullptr != _gProfilerPtr_Process);
    Assert(nullptr != _gProfilerPtr_Thread);

    _gProfilerPtr_Global->~FProfiler();
    _gProfilerPtr_Process->~FProfiler();
    _gProfilerPtr_Thread->~FProfiler();

    _gProfilerPtr_Global = nullptr;
    _gProfilerPtr_Process = nullptr;
    _gProfilerPtr_Thread = nullptr;
}
//----------------------------------------------------------------------------
const FProfiler* FProfiler::Global()  { return _gProfilerPtr_Global; }
const FProfiler* FProfiler::Process() { return _gProfilerPtr_Process; }
const FProfiler* FProfiler::Thread()  { return _gProfilerPtr_Thread; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FProfilingScope::FProfilingScope(const FProfiler* profiler, u32 marker, const char* comment)
:   _profiler(profiler) {
    if (_profiler) {
        _profiler->Start();
        _profiler->MarkAndComment(marker, comment);
    }
}
//----------------------------------------------------------------------------
FProfilingScope::~FProfilingScope() {
    if (_profiler)
        _profiler->Stop();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core*

#endif //!WITH_CORE_PROFILING
