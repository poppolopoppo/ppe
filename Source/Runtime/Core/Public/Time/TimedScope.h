#pragma once

#include "Core.h"

#include "Time/Timepoint.h"

#include "Diagnostic/Benchmark.h"

#if USE_PPE_BENCHMARK
#   include "IO/StringView.h"
#   define BENCHMARK_SCOPE(_CATEGORY, _MSG) \
        const ::PPE::FBenchmarkScope ANONYMIZE(_benchmarkScope)((_CATEGORY), (_MSG))
#   define IOBENCHMARK_SCOPE(_CATEGORY, _MSG, _SIZE_IN_BYTES_PTR) \
        const ::PPE::FIOBenchmarkScope ANONYMIZE(_IObenchmarkScope)((_CATEGORY), (_MSG), (_SIZE_IN_BYTES_PTR))
#else
#   define BENCHMARK_SCOPE(_CATEGORY, _MSG) NOOP()
#   define IOBENCHMARK_SCOPE(_CATEGORY, _MSG, _SIZE_IN_BYTES_PTR) NOOP()
#endif

namespace PPE {
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
#if USE_PPE_BENCHMARK
class FBenchmarkScope : public FTimedScope {
public:
    FBenchmarkScope(const FWStringView& category, const FWStringView& message);
    ~FBenchmarkScope();

private:
    const FWStringView _category;
    const FWStringView _message;
    FBenchmarkScope* const _parentIFP;
    const size_t _depth;
    FTimespan _accumulated;
};
#endif //!USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
#if USE_PPE_BENCHMARK
class FIOBenchmarkScope : public FTimedScope {
public:
    FIOBenchmarkScope(const FWStringView& category, const FWStringView& message, const std::streamsize* pSizeInBytes);
    ~FIOBenchmarkScope();

private:
    const FWStringView _category;
    const FWStringView _message;
    const std::streamsize* _pSizeInBytes;
};
#endif //!USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
