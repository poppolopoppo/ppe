#include "stdafx.h"

#include "TimedScope.h"

#if USE_PPE_BENCHMARK
#   include "Diagnostic/Logger.h"
#   include "IO/FormatHelpers.h"
#   include "Maths/Units.h"

namespace PPE {
LOG_CATEGORY_VERBOSITY(PPE_API, Benchmark, NoDebug)
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

    LOG(Benchmark, Info, L"{0:28} | {1}{2} | {3:10f2} / {4:12f2}",
        _category,
        Fmt::Repeat(L"  ", _depth),
        Fmt::PadRight(_message, 28 - _depth * 2, L' '),
        Fmt::DurationInMs(elapsed),
        Fmt::Ternary(*_accumulated > 0, _accumulated, L'-') );
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

    LOG(Benchmark, Info, L" {0:20} | {2:10f2} | {3:10f2} = {4:10f2} Mb/s | {1}",
        _category,
        _message,
        Fmt::DurationInMs(elapsed),
        Fmt::SizeInBytes(checked_cast<size_t>(*_pSizeInBytes)),
        FMegabytes(FBytes((double)*_pSizeInBytes)).Value() / FSeconds(elapsed).Value() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!USE_PPE_BENCHMARK
