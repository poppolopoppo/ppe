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
class FProfiler {
public:
    enum ELevel : int {
        GlobalLevel     = 1,
        ProcessLevel    = 2,
        ThreadLevel     = 3,
    };

    static constexpr u32 CurrentID = -1;

private:
    explicit FProfiler(ELevel level, u32 id = CurrentID);
public:
    ~FProfiler();

    FProfiler(const FProfiler& other) = delete;
    FProfiler& operator=(const FProfiler& other) = delete;

    void Start() const;
    void Stop() const;

    void Resume() const;
    void Suspend() const;

    void Mark(u32 data) const;
    void MarkAndComment(u32 data, const char* cstr) const;

    static void FStartup();
    static void Shutdown();

    static const FProfiler* Global();
    static const FProfiler* Process();
    static const FProfiler* Thread();

private:
    ELevel _level;
    u32 _id;
};
//----------------------------------------------------------------------------
class FProfilingScope {
public:
    FProfilingScope(const FProfiler* profiler, u32 marker, const char* comment);
    ~FProfilingScope();

private:
    const FProfiler* _profiler;
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
    const Core::FProfilingScope _profilingScope_##__LINE__( \
        Core::FProfiler::_Level(), \
        (_Marker), \
        _Message ", " __FILE__ "(" STRINGIZE(__LINE__) ")" )
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define PROFILING_SCOPE(_Level, _Marker, _Message) NOOP
//----------------------------------------------------------------------------
#endif //!WITH_CORE_PROFILING
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
