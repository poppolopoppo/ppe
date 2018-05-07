#pragma once

#include "Core/Core.h"

#include "Core/Time/Timepoint.h"

#if defined(FINAL_RELEASE)
#   define USE_CORE_BENCHMARK 0
#else
#   define USE_CORE_BENCHMARK 1
#endif

#if USE_CORE_BENCHMARK
#   include "Core/IO/StringView.h"
#   define BENCHMARK_SCOPE(_CATEGORY, _MSG) \
        const ::Core::FBenchmarkScope ANONYMIZE(_benchmarkScope)((_CATEGORY), (_MSG))
#   define IOBENCHMARK_SCOPE(_CATEGORY, _MSG, _SIZE_IN_BYTES_PTR) \
        const ::Core::FIOBenchmarkScope ANONYMIZE(_IObenchmarkScope)((_CATEGORY), (_MSG), (_SIZE_IN_BYTES_PTR))
#else
#   define BENCHMARK_SCOPE(_CATEGORY, _MSG) NOOP()
#   define IOBENCHMARK_SCOPE(_CATEGORY, _MSG, _SIZE_IN_BYTES_PTR) NOOP()
#endif

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
#if USE_CORE_BENCHMARK
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
#endif //!USE_CORE_BENCHMARK
//----------------------------------------------------------------------------
#if USE_CORE_BENCHMARK
class FIOBenchmarkScope : public FTimedScope {
public:
    FIOBenchmarkScope(const FWStringView& category, const FWStringView& message, const std::streamsize* pSizeInBytes);
    ~FIOBenchmarkScope();

private:
    const FWStringView _category;
    const FWStringView _message;
    const std::streamsize* _pSizeInBytes;
};
#endif //!USE_CORE_BENCHMARK
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
