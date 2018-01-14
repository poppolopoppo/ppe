#include "stdafx.h"

#include "TimedScope.h"

#if USE_CORE_BENCHMARK
#   include "Diagnostic/Logger.h"
#   include "IO/FormatHelpers.h"
#   include "Maths/Units.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static THREAD_LOCAL FBenchmarkScope* GBenchmarkLastScopeTLS_ = nullptr;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBenchmarkScope::FBenchmarkScope(const FWStringView& category, const FWStringView& message)
    : _category(category)
    , _message(message)
    , _parentIFP(GBenchmarkLastScopeTLS_)
    , _depth(_parentIFP ? _parentIFP->_depth + 1 : 0) {
    GBenchmarkLastScopeTLS_ = this;
}
//----------------------------------------------------------------------------
FBenchmarkScope::~FBenchmarkScope() {
    const FTimespan elapsed = Elapsed();

    Assert(this == GBenchmarkLastScopeTLS_);
    GBenchmarkLastScopeTLS_ = _parentIFP;

    if (_parentIFP)
        _parentIFP->_accumulated.SetValue(*_parentIFP->_accumulated + *elapsed);

    LOG(Profiling, L"{0:28} | {1}{2} | {3:8f2} / {4:8f2}",
        _category,
        Fmt::Repeat(L"  ", _depth),
        Fmt::PadRight(_message, 30 - _depth * 2, L' '),
        elapsed,
        Fmt::Ternary(*_accumulated > 0, _accumulated, '-') );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FIOBenchmarkScope::FIOBenchmarkScope(const FWStringView& category, const FWStringView& message, const std::streamsize* pSizeInBytes)
    : _category(category)
    , _message(message)
    , _pSizeInBytes(pSizeInBytes) {
    Assert(pSizeInBytes);
}
//----------------------------------------------------------------------------
FIOBenchmarkScope::~FIOBenchmarkScope() {
    const FTimespan elapsed = Elapsed();

    LOG(Profiling, L" {0:20} | {2:8} | {3:10} = {4:10f2} Mb/s | {1}",
        _category,
        _message,
        elapsed,
        Fmt::FSizeInBytes{ checked_cast<size_t>(*_pSizeInBytes) },
        FMegabytes(FBytes((double)*_pSizeInBytes)).Value() / FSeconds(elapsed).Value() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif //!USE_CORE_BENCHMARK
