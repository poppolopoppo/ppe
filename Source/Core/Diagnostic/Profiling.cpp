#include "stdafx.h"

#include "Profiling.h"

#ifdef WITH_CORE_PROFILING

#ifdef OS_WINDOWS
#   include <VSPerf.h>
#else
#   error "unsuppored platform"
#endif

#include "Memory/AlignedStorage.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(::PROFILE_GLOBALLEVEL   == Profiler::GlobalLevel);
STATIC_ASSERT(::PROFILE_PROCESSLEVEL  == Profiler::ProcessLevel);
STATIC_ASSERT(::PROFILE_THREADLEVEL   == Profiler::ThreadLevel);
//----------------------------------------------------------------------------
STATIC_ASSERT(::PROFILE_CURRENTID     == Profiler::CurrentID);
//----------------------------------------------------------------------------
Profiler::Profiler(Level level, u32 id /* = CurrentID */) : _level(level), _id(id) {}
//----------------------------------------------------------------------------
Profiler::~Profiler() {}
//----------------------------------------------------------------------------
void Profiler::Start() const {
    ::StartProfile(::PROFILE_CONTROL_LEVEL(_level), _id);
}
//----------------------------------------------------------------------------
void Profiler::Stop() const {
    ::StopProfile(::PROFILE_CONTROL_LEVEL(_level), _id);
}
//----------------------------------------------------------------------------
void Profiler::Resume() const {
    ::ResumeProfile(::PROFILE_CONTROL_LEVEL(_level), _id);
}
//----------------------------------------------------------------------------
void Profiler::Suspend() const {
    ::SuspendProfile(::PROFILE_CONTROL_LEVEL(_level), _id);
}
//----------------------------------------------------------------------------
void Profiler::Mark(u32 data) const {
    ::MarkProfile(data);
}
//----------------------------------------------------------------------------
void Profiler::MarkAndComment(u32 data, const char* cstr) const {
    ::CommentMarkProfileA(data, cstr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static POD_STORAGE(Profiler) _gProfilerPOD_Global;
static Profiler* _gProfilerPtr_Global = nullptr;
static POD_STORAGE(Profiler) _gProfilerPOD_Process;
static Profiler* _gProfilerPtr_Process = nullptr;
static POD_STORAGE(Profiler) _gProfilerPOD_Thread;
static Profiler* _gProfilerPtr_Thread = nullptr;
}
//----------------------------------------------------------------------------
void Profiler::Startup() {
    Assert(nullptr == _gProfilerPtr_Global);
    Assert(nullptr == _gProfilerPtr_Process);
    Assert(nullptr == _gProfilerPtr_Thread);

    _gProfilerPtr_Global = new ((void*)&_gProfilerPOD_Global) Profiler(Profiler::GlobalLevel);
    _gProfilerPtr_Process = new ((void*)&_gProfilerPOD_Process) Profiler(Profiler::ProcessLevel);
    _gProfilerPtr_Thread = new ((void*)&_gProfilerPOD_Thread) Profiler(Profiler::ThreadLevel);

    _gProfilerPtr_Global->Stop(); // profiler is started by default
}
//----------------------------------------------------------------------------
void Profiler::Shutdown() {
    Assert(nullptr != _gProfilerPtr_Global);
    Assert(nullptr != _gProfilerPtr_Process);
    Assert(nullptr != _gProfilerPtr_Thread);

    _gProfilerPtr_Global->~Profiler();
    _gProfilerPtr_Process->~Profiler();
    _gProfilerPtr_Thread->~Profiler();

    _gProfilerPtr_Global = nullptr;
    _gProfilerPtr_Process = nullptr;
    _gProfilerPtr_Thread = nullptr;
}
//----------------------------------------------------------------------------
const Profiler* Profiler::Global()  { return _gProfilerPtr_Global; }
const Profiler* Profiler::Process() { return _gProfilerPtr_Process; }
const Profiler* Profiler::Thread()  { return _gProfilerPtr_Thread; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ProfilingScope::ProfilingScope(const Profiler* profiler, u32 marker, const char* comment)
:   _profiler(profiler) {
    if (_profiler) {
        _profiler->Start();
        _profiler->MarkAndComment(marker, comment);
    }
}
//----------------------------------------------------------------------------
ProfilingScope::~ProfilingScope() {
    if (_profiler)
        _profiler->Stop();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core*

#endif //!WITH_CORE_PROFILING
