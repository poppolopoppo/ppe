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
#ifndef FINAL_RELEASE
class FBenchmarkScope : public FTimedScope {
public:
    FBenchmarkScope(const wchar_t* category, const wchar_t* message) 
        : _category(category)
        , _message(message) {}

    ~FBenchmarkScope() { 
        LOG(Profiling, L" benchmark - {0:-30} {1:-30} = {2}", 
            _category, _message, Elapsed()); 
    }

private:
    const wchar_t* const _category;
    const wchar_t* const _message;
};
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
