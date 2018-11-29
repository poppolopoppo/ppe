#pragma once

#include "Core_fwd.h"

#define USE_PPE_BENCHMARK (!USE_PPE_FINAL_RELEASE)

#if USE_PPE_BENCHMARK

#include "Container/Array.h"
#include "Container/Tuple.h"
#include "Container/TupleHelpers.h"
#include "Container/Vector.h"

#include "Diagnostic/Logger.h"

#include "Maths/RandomGenerator.h"
#include "Maths/Units.h"
#include "Maths/VarianceEstimator.h"

#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformTime.h"

#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

#include <algorithm>

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, Benchmark);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBenchmark {
public:
    static constexpr u32 ReservoirSize = 100;
    static constexpr u32 MinIterations = ReservoirSize*3;

    FStringView Name{ "none" };
    u32 BatchSize{ 1 };
#ifdef _DEBUG
    u32 MaxIterations{ 50000 };
#else
    u32 MaxIterations{ 500000 };
#endif
    double MaxVarianceError{ 1e-3 };

public: // Helpers
    template <class Tp>
    static FORCE_INLINE void DoNotOptimize(Tp const& value) {
        UseCharPointer_(&reinterpret_cast<char const volatile&>(value));
        FPlatformAtomics::MemoryBarrier();
    }

    template <class Tp>
    static FORCE_INLINE void DoNotOptimizeLoop(Tp const& value) {
        UseCharPointer_(&reinterpret_cast<char const volatile&>(value));
    }

    static FORCE_INLINE void ClobberMemory() {
        FPlatformAtomics::MemoryBarrier();
    }

private:
    static NO_INLINE void UseCharPointer_(char const volatile*);

private: // FTimer
    enum ECounterType {
        CpuCycles,
        PerfCounter,
        ChronoTime,
    };

    template <ECounterType _Type> struct TCounter;

    template <> struct TCounter<CpuCycles> {
        using date_type = u64;
        static const char* Units() { return "k.cycles"; }
        static date_type Now() { return FPlatformTime::ThreadCpuCycles(); }
        static double ElapsedSince(date_type start) { return static_cast<double>(Now() - start) * 0.001; }
    };
    template <> struct TCounter<PerfCounter> {
        using date_type = i64;
        static const char* Units() { return "µs"; }
        static date_type Now() { return FPlatformTime::Cycles(); }
        static double ElapsedSince(date_type start) { return FPlatformTime::ToMicroseconds(Now() - start); }
    };
    template <> struct TCounter<ChronoTime> {
        using date_type = double;
        static const char* Units() { return "µs"; }
        static date_type Now() { return FPlatformTime::ChronoMicroseconds(); }
        static double ElapsedSince(date_type start) { return static_cast<double>(Now() - start); }
    };

    using FCpuCycles = TCounter<CpuCycles>;
    using FPerfCounter = TCounter<PerfCounter>;
    using FChronoTime = TCounter<ChronoTime>;

    using FCounter = TCounter<CpuCycles>;

    struct FTimer {
        FCounter::date_type StartedAt{ 0 };
        double Elapsed{ 0 };
        void Start() {
            Assert_NoAssume(0 == StartedAt);
            ClobberMemory();
            StartedAt = FCounter::Now();
        }
        void Stop() {
            Elapsed += FCounter::ElapsedSince(StartedAt);
            Assert_NoAssume(0 != StartedAt);
            StartedAt = 0;
        }
        double Reset() {
            Assert_NoAssume(0 == StartedAt);
            const double t = Elapsed;
            Elapsed = 0;
            return t;
        }
    };

public: // FState
    struct FRun;
    class FIterator;
    class FState {
    public:
        FState(const FBenchmark& benchmark)
            : _benchmark(benchmark)
        {}

        FIterator begin();
        FIterator end();

        const FVarianceEstimator& Estimator() const { return _estimator; }
        TMemoryView<const double> Reservoir() const { return _reservoir; }

        double Mean() const { return _estimator.Mean; }

        // must have call Finish()
        double Low() const { return _reservoir[ReservoirSize/10]; } // 10th percentile
        double High() const { return _reservoir[(9*ReservoirSize)/10]; } // 90th percentile

        void PauseTiming() { _timer.Stop(); }
        void ResumeTiming() { _timer.Start(); }

        void Finish() {
            // sort the reservoir to deduce the percentiles
            std::sort(std::begin(_reservoir), std::end(_reservoir));

            // log some progress
            LOG(Benchmark, Info,
                L"{0:/18}: {1:10f4} < {2:10f4} < {3:10f4} µs [{4:6} iterations] -> {5}",
                _benchmark.Name,
                Low(), Mean(), High(),
                _estimator.Count,
                _estimator.SampleVariance());
        }

    protected:
        friend class FIterator;

        u32 LoopCount() const {
             return (_benchmark.BatchSize * ReservoirSize);
        }

        void Start(u32 loop) {
            if ((loop & (_benchmark.BatchSize - 1)) == 0) {
                _timer.Start();
            }
        }

        void Stop(u32 loop) {
            if (((loop - 1) & (_benchmark.BatchSize - 1)) == 0) {
                _timer.Stop();

                const double t = _timer.Reset();

                if (_estimator.Count < ReservoirSize)
                    _reservoir[_estimator.Count] = t;
                else {
                    const size_t r = size_t(_rnd.NextU64(_estimator.Count + 1));
                    if (r < ReservoirSize)
                        _reservoir[r] = t;
                }

                _estimator.Add(t);
            }
        }

        NO_INLINE u32 RemainingIterations() const {
            return (
                (_estimator.Count < MinIterations) ||
                (_estimator.Count < _benchmark.MaxIterations && VarianceError() > _benchmark.MaxVarianceError)
                ? LoopCount() : 0 );
        }

    private:
        const FBenchmark& _benchmark;
        FTimer _timer;
        FVarianceEstimator _estimator;
        FRandomGenerator _rnd;
        double _reservoir[ReservoirSize];

        double VarianceError() const {
            const double variance = _estimator.Variance();
            const double sampleVariance = _estimator.SampleVariance();

            return std::sqrt(std::abs(variance - sampleVariance)) / _estimator.Mean;
        }
    };

public: // FIterator
    class FIterator {
    public:
        struct value_type {/* unused */ };
        using iterator_category = std::forward_iterator_tag;
        using pointer = value_type;
        using reference = value_type;
        using difference_type = std::ptrdiff_t;

        explicit FIterator(FState& state)
            : _state(&state)
            , _loop(_state->LoopCount())
        {}

        FORCE_INLINE value_type operator*() const { return value_type{}; }
        FORCE_INLINE FIterator& operator++() {
            _state->Stop(_loop);
            Assert(_loop);
            --_loop;
            return *this;
        }

        FORCE_INLINE bool operator!=(const FIterator&) const {
            if (0 == _loop) _loop = _state->RemainingIterations();
            if (0 == _loop) return false;

            _state->Start(_loop);
            return true;
        }

    private:
        FState* _state;
        mutable u32 _loop;
    };

public: // FRun
    struct FRun {
        u64 NumIterations{ 0 };
        double Mean{ 0 }, Low{ 0 }, High{ 0 };
    };

    template <typename _Benchmark, typename... _Args>
    static FRun Run(_Benchmark& bm, const _Args&... args) {
        FState state{ bm };
        bm(state, args...);
        state.Finish();
        return FRun{
            state.Estimator().Count,
            state.Mean(),
            state.Low(),
            state.High() };
    }

public: // TTable<>
    template <typename... _Benchmarks>
    struct TTable {
        static constexpr size_t Dim = sizeof...(_Benchmarks);
        using FHeaders = TTuple<_Benchmarks...>;
        using FRow = TArray<FRun, Dim>;

        struct FEntry {
            FStringView Name;
            FRow Row;
        };

        using FEntries = VECTORINSITU(Diagnostic, FEntry, 8);

        FStringView Name;
        FHeaders Headers;
        FEntries Entries;

        size_t dim() const { return Dim; }

        TTable(const FStringView& name, _Benchmarks&&... headers)
            : Name(name)
            , Headers(std::forward<_Benchmarks>(headers)...)
        {}

        void Add(const FStringView& name, FRow&& row) {
            Entries.emplace_back(name, std::move(row));
        }

        template <typename... _Args>
        void Run(const FStringView& name, const _Args&... args) {
            LOG(Benchmark, Emphasis, L"running {0} benchmarks for <{1}> ...", Dim, name);
            Entries.emplace_back(name, MapTuple<FRun>(Headers, [&args...](const auto& bench) {
                return FBenchmark::Run(bench, args...);
            }));
        }

        template <typename _Lambda>
        void ForeachHeader(_Lambda foreach) const {
            ForeachTuple(Headers, foreach);
        }

        template <typename _Lambda>
        void ForeachEntry(_Lambda foreach) const {
            std::for_each(Entries.begin(), Entries.end(), foreach);
        }

    };

    template <typename... _Benchmarks>
    static TTable<_Benchmarks...> MakeTable(const FStringView& name, _Benchmarks&&... benchmarks) {
        return TTable<_Benchmarks...>(name, std::forward<_Benchmarks>(benchmarks)...);
    }

public: // export table results
    template <typename... _Benchmarks>
    static void WTxt(const TTable<_Benchmarks...>& table, FWTextWriter& oss) {
        oss << L"Benchmark table <" << table.Name << ">, units = " << FCounter::Units() << " :" << Eol;

        constexpr size_t stride = 20;

        oss << L'|'
            << FTextFormat::Trunc(stride, L' ')
            << table.Name;

        table.ForeachHeader([&oss, stride](const auto& x) {
            oss << L'|'
                << FTextFormat::Trunc(stride, L' ')
                << x.Name;
        });

        oss << L'|' << Eol;

        oss << Fmt::Repeat(L"-", 21 * (table.dim() + 1/* name */)) << Eol;

        table.ForeachEntry([&oss, stride](const auto& x) {
            oss << L'|'
                << FTextFormat::Trunc(stride, L' ')
                << x.Name;

            for (const auto& c : x.Row)
                oss << L'|'
                    << FTextFormat::Trunc(stride, L' ')
                    << FTextFormat::Float(FTextFormat::FixedFloat, 6)
                    << c.Mean;

            oss << L'|' << Eol;
        });
    }

    template <typename... _Benchmarks>
    static void Log(const TTable<_Benchmarks...>& table) {
        FWStringBuilder sb;
        WTxt(table, sb);
        FLogger::Log(
            GLogCategory_Benchmark,
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            sb.ToString() );
    }

    template <typename... _Benchmarks>
    static void Csv(const TTable<_Benchmarks...>& table, FTextWriter& oss) {
        oss << table.Name;

        table.ForeachHeader([&oss](const auto& x) {
            oss << ';' << x.Name;
        });

        oss << Eol;

        table.ForeachEntry([&oss](const auto& x) {
            oss << x.Name;
            for (const auto& c : x.Row)
                oss << ';' << c.Mean;
            oss << Eol;
        });
    }
};
//----------------------------------------------------------------------------
inline FBenchmark::FIterator FBenchmark::FState::begin() { return FIterator{ *this }; }
//----------------------------------------------------------------------------
inline FBenchmark::FIterator FBenchmark::FState::end() { return FIterator{ *this }; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!!(USE_PPE_FINAL_RELEASE)
