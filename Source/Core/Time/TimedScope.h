#pragma once

#include "Core/Core.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Time/Timepoint.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTimedScope {
public:
    FTimedScope() : _startedAt(FTimepoint::Now()) {}

    const FTimepoint& StartedAt() const { return _startedAt; }
    FTimespan Elapsed() const { return FTimepoint::ElapsedSince(_startedAt); }

private:
    FTimepoint _startedAt;
};
//----------------------------------------------------------------------------
class FBenchmarkScope : public FTimedScope {
public:
    FBenchmarkScope(const char* message) : _message(message) {}
    ~FBenchmarkScope() { LOG(Profiling, L"benchmark scope: {0:-30} = {1}", _message, Elapsed()); }

private:
    const char* _message;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
