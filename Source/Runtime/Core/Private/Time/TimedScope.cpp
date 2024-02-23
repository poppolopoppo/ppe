// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Time/TimedScope.h"

#if USE_PPE_BENCHMARK
#   include "Diagnostic/Logger.h"
#   include "IO/FormatHelpers.h"
#   include "IO/TextWriter.h"
#   include "Maths/Units.h"

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, Benchmark)
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
FBenchmarkScope::FBenchmarkScope(FWStringLiteral category, const FWStringView& message)
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

    PPE_LOG(Benchmark, Profiling, "{0:28} | {1}{2} | {3:10f2} / {4:12f2}",
        _category,
        Fmt::Repeat(' ', _depth * 2),
        Fmt::PadRight(_message, 28 - _depth * 2, ' '),
        Fmt::DurationInMs(elapsed),
        Fmt::Ternary(*_accumulated > 0, _accumulated, '-') );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FIOBenchmarkScope::FIOBenchmarkScope(FWStringLiteral category, const FWStringView& message, const std::streamsize* pSizeInBytes)
    : _category(category)
    , _message(message)
    , _pSizeInBytes(pSizeInBytes) {
    Assert(pSizeInBytes);
}
//----------------------------------------------------------------------------
FIOBenchmarkScope::~FIOBenchmarkScope() {
#if USE_PPE_LOGGER
    const FTimespan elapsed = Elapsed();

    PPE_LOG(Benchmark, Profiling, " {0:20} | {2:10f2} | {3:10f2} = {4:10f2} Mb/s | {1}",
        _category,
        _message,
        Fmt::DurationInMs(elapsed),
        Fmt::SizeInBytes(checked_cast<size_t>(*_pSizeInBytes)),
        FMegabytes(FBytes((double)*_pSizeInBytes)).Value() / FSeconds(elapsed).Value() );
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!USE_PPE_BENCHMARK
