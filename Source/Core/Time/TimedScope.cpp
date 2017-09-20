#include "stdafx.h"

#include "TimedScope.h"

#ifndef FINAL_RELEASE
#   include "Diagnostic/Logger.h"
#   include "IO/FormatHelpers.h"
#   include "Maths/Units.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static THREAD_LOCAL size_t GBenchmarkScopeDepth_ = 0;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBenchmarkScope::FBenchmarkScope(const wchar_t* category, const wchar_t* message)
    : _category(category)
    , _message(message) {
    ++GBenchmarkScopeDepth_;
}
//----------------------------------------------------------------------------
FBenchmarkScope::~FBenchmarkScope() {
    Assert(GBenchmarkScopeDepth_);
    GBenchmarkScopeDepth_--;

    LOG(Profiling, L"{0:28} | {1}{2} | {3:8f2}",
        _category,
        Repeat(L"  ", GBenchmarkScopeDepth_),
        PadRight(_message, 30 - GBenchmarkScopeDepth_ * 2, L' '),
        Elapsed() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FIOBenchmarkScope::FIOBenchmarkScope(const wchar_t* category, const wchar_t* message, size_t sizeInBytes)
    : _category(category)
    , _message(message)
    , _sizeInBytes(sizeInBytes) {
    ++GBenchmarkScopeDepth_;
}
//----------------------------------------------------------------------------
FIOBenchmarkScope::~FIOBenchmarkScope() {
    Assert(GBenchmarkScopeDepth_);
    GBenchmarkScopeDepth_--;

    const FTimespan elapsed = Elapsed();

    LOG(Profiling, L" {0:20} | {3:8} | {4:10} = {5:10f2} Mb/s | {1}{2}",
        _category,
        Repeat(L"  ", GBenchmarkScopeDepth_),
        PadRight(_message, 30 - GBenchmarkScopeDepth_ * 2, L' '),
        Elapsed(),
        FSizeInBytes{ _sizeInBytes },
        FMegabytes(FBytes((double)_sizeInBytes)).Value() / FSeconds(elapsed).Value() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif //!FINAL_RELEASE
