#pragma once

#include "Core_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "Time/Timepoint.h"

#define USE_PPE_BENCHMARK (!USE_PPE_FINAL_RELEASE && USE_PPE_LOGGER)

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
class PPE_CORE_API FTimedScope {
public:
    FTimedScope() NOEXCEPT : _startedAt(FTimepoint::Now()) {}

    const FTimepoint& StartedAt() const { return _startedAt; }
    FTimespan Elapsed() const { return FTimepoint::ElapsedSince(_startedAt); }
    FTimepoint::value_type TotalTicks() const { return (FTimepoint::Now().Value() - _startedAt.Value()); }

private:
    FTimepoint _startedAt;
};
//----------------------------------------------------------------------------
class FMovingAverageTimer {
public:
    FTimespan Average = -1;
    FTimespan Min = -1;
    FTimespan Max = -1;
    FTimespan Last = -1;

    struct FScope {
        FMovingAverageTimer& Timer;
        FTimedScope TimedScope;

        explicit FScope(FMovingAverageTimer& timer) NOEXCEPT : Timer(timer) {}
        ~FScope() NOEXCEPT {
            Timer.Update(TimedScope.Elapsed());
        }
    };

    void Update(const FTimedScope& scope, int n = 30) NOEXCEPT {
        Update(scope.Elapsed(), n);
    }
    void Update(const FTimespan& elapsed, int n = 30) NOEXCEPT {
        Assert(elapsed >= 0);
        Assert(n > 0);

        if (Average < 0) {
            Average = Min = Max = Last = elapsed;
            return;
        }

        // exponential moving average
        double avg = Average.Value();
        avg -= avg / n;
        avg += elapsed.Value() / n;
        Average.SetValue(avg);

        if (elapsed < Min)
            Min = elapsed;

        if (elapsed > Max)
            Max = elapsed;

        Last = elapsed;
    }
};
//----------------------------------------------------------------------------
class PPE_CORE_API FAtomicTimedScope {
public:
    explicit FAtomicTimedScope(std::atomic<u64>* pTicks) NOEXCEPT
    :   _pTicks(pTicks)
    {}

    ~FAtomicTimedScope() NOEXCEPT {
        _pTicks->fetch_add(_timer.TotalTicks(), std::memory_order_relaxed);
    }

private:
    FTimedScope _timer;
    std::atomic<u64>* _pTicks;
};
//----------------------------------------------------------------------------
#if USE_PPE_BENCHMARK
class PPE_CORE_API FBenchmarkScope : public FTimedScope {
public:
    FBenchmarkScope(FWStringLiteral category, const FWStringView& message);
    ~FBenchmarkScope();

private:
    const FWStringLiteral _category;
    const FWStringView _message;
    FBenchmarkScope* const _parentIFP;
    const size_t _depth;
    FTimespan _accumulated;
};
#endif //!USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
#if USE_PPE_BENCHMARK
class PPE_CORE_API FIOBenchmarkScope : public FTimedScope {
public:
    FIOBenchmarkScope(FWStringLiteral category, const FWStringView& message, const std::streamsize* pSizeInBytes);
    ~FIOBenchmarkScope();

private:
    const FWStringLiteral _category;
    const FWStringView _message;
    const std::streamsize* _pSizeInBytes;
};
#endif //!USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
