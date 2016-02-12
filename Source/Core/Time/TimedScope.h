#pragma once

#include "Core/Core.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Time/Timepoint.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TimedScope {
public:
    TimedScope() : _startedAt(Timepoint::Now()) {}

    const Timepoint& StartedAt() const { return _startedAt; }
    Timespan Elapsed() const { return Timepoint::ElapsedSince(_startedAt); }

private:
    Timepoint _startedAt;
};
//----------------------------------------------------------------------------
class BenchmarkScope : public TimedScope {
public:
    BenchmarkScope(const char* message) : _message(message) {}
    ~BenchmarkScope() { LOG(Profiling, L"benchmark scope: {0:-30} = {1}", _message, Elapsed()); }

private:
    const char* _message;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
