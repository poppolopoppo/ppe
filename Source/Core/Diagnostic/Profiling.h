#pragma once

#include "Core/Core.h"

#if not defined(FINAL_RELEASE) && 0
#   define WITH_CORE_PROFILING
#endif

#ifdef WITH_CORE_PROFILING
namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Profiler {
public:
    enum Level : int {
        GlobalLevel     = 1,
        ProcessLevel    = 2,
        ThreadLevel     = 3,
    };

    static constexpr u32 CurrentID = -1;

private:
    explicit Profiler(Level level, u32 id = CurrentID);
public:
    ~Profiler();

    Profiler(const Profiler& other) = delete;
    Profiler& operator=(const Profiler& other) = delete;

    void Start() const;
    void Stop() const;

    void Resume() const;
    void Suspend() const;

    void Mark(u32 data) const;
    void MarkAndComment(u32 data, const char* cstr) const;

    static void Startup();
    static void Shutdown();

    static const Profiler* Global();
    static const Profiler* Process();
    static const Profiler* Thread();

private:
    Level _level;
    u32 _id;
};
//----------------------------------------------------------------------------
class ProfilingScope {
public:
    ProfilingScope(const Profiler* profiler, u32 marker, const char* comment);
    ~ProfilingScope();

private:
    const Profiler* _profiler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif //!WITH_CORE_PROFILING

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_PROFILING
//----------------------------------------------------------------------------
#define PROFILING_SCOPE(_Level, _Marker, _Message) \
    const Core::ProfilingScope _profilingScope_##__LINE__( \
        Core::Profiler::_Level(), \
        (_Marker), \
        _Message ", " __FILE__ "(" STRINGIZE(__LINE__) ")" )
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define PROFILING_SCOPE(_Level, _Message) NOOP
//----------------------------------------------------------------------------
#endif //!WITH_CORE_PROFILING
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

