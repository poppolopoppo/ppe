#pragma once

#include "Core_fwd.h"
#include "Meta/Utility.h"

#include "Time/TimedScope.h"

#if USE_PPE_BENCHMARK

#include "Container/Array.h"
#include "Container/Tuple.h"
#include "Container/TupleHelpers.h"
#include "Container/Vector.h"

#include "Diagnostic/Logger.h"

#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformTime.h"

#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

#include "Maths/Threefy.h"
#include "Maths/Units.h"
#include "Maths/VarianceEstimator.h"

#include "Thread/Task/TaskHelpers.h"
#include "Thread/ThreadPool.h"

#include <algorithm>

#if (!defined(__GNUC__) && !defined(__clang__)) || defined(__pnacl__) || defined(__EMSCRIPTEN__)
#   define USE_BENCHMARK_ASM_DONOTOPTIMIZE (0)
#else
#   define USE_BENCHMARK_ASM_DONOTOPTIMIZE (1)
#endif

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, Benchmark);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBenchmark {
public:
    static constexpr u32 HistogramSize = 20;
    static constexpr u32 ReservoirSize = 200;

    using FApproximateHistogram = TApproximateHistogram<double, HistogramSize, ReservoirSize>;

    FStringView Name{ "none" };
    u32 InputDim{ 1 };
    u32 BatchSize{ 1 };
#if USE_PPE_ASSERT
    u32 MinIterations{ FApproximateHistogram::MinSamples };
    u32 MaxIterations{ 5000 };
    double MaxVarianceError{ 1e-2 };
#else
    u32 MinIterations{ Max(FApproximateHistogram::MinSamples * 2, 5000_u32) };
    u32 MaxIterations{ 1000000 };
    double MaxVarianceError{ 1e-3 };
#endif
    u32 RandomSeed{ PPE_HASH_VALUE_SEED_32 };

public: // Helpers
#if USE_BENCHMARK_ASM_DONOTOPTIMIZE
    template <class Tp>
    static FORCE_INLINE void DoNotOptimize(Tp& value) NOEXCEPT {
#   if defined(__clang__)
        asm volatile("" : "+r,m"(value) : : "memory");
#   else
        asm volatile("" : "+m,r"(value) : : "memory");
#   endif
    }

    template <class Tp>
    static FORCE_INLINE void DoNotOptimize(Tp const& value) NOEXCEPT {
        asm volatile("" : : "r,m"(value) : "memory");
    }

    static FORCE_INLINE void ClobberMemory() NOEXCEPT {
        asm volatile("" : : : "memory");
    }
#else
    template <class Tp>
    static FORCE_INLINE void DoNotOptimize(Tp const& value) NOEXCEPT {
        UseCharPointer_(&reinterpret_cast<char const volatile&>(value));
        FPlatformAtomics::MemoryBarrier();
    }

    static FORCE_INLINE void ClobberMemory() NOEXCEPT {
        FPlatformAtomics::MemoryBarrier();
    }
#endif //!USE_BENCHMARK_ASM_DONOTOPTIMIZE

private:
    PPE_CORE_API static NO_INLINE void UseCharPointer_(char const volatile*) NOEXCEPT;

private: // FTimer
    enum ECounterType {
        RawTicks,
        ThreadCycles,
        PerfCounter,
        ChronoTime,
    };

    template <ECounterType _Type> struct TCounter;

    template <> struct TCounter<RawTicks> {
        using date_type = u64;
        static CONSTEXPR FWStringView Units() { return L"ticks"; }
        date_type Now() const NOEXCEPT { return FPlatformTime::Rdtsc(); }
        double ElapsedSince(date_type start) const NOEXCEPT { return static_cast<double>(Now() - start) * 0.1; }
    };
    // template <> struct TCounter<ThreadCycles> {
    //     using date_type = u64;
    //     static CONSTEXPR FWStringView Units() { return L"k.cycles"; }
    //     date_type Now() const NOEXCEPT { return FPlatformTime::CpuTime(); }
    //     double ElapsedSince(date_type start) const NOEXCEPT { return static_cast<double>(Now() - start) * 0.001; }
    // };
    template <> struct TCounter<PerfCounter> {
        using date_type = i64;
        static CONSTEXPR FWStringView Units() { return L"ns"; }
        date_type Now() const NOEXCEPT { return FPlatformTime::Cycles(); }
        double ElapsedSince(date_type start) const NOEXCEPT {
            return FPlatformTime::ToSeconds(Now() - start) * 1000000;
        }
    };
    template <> struct TCounter<ChronoTime> {
        using date_type = double;
        static CONSTEXPR FWStringView Units() { return L"µs"; }
        date_type Now() const { return FPlatformTime::ChronoMicroseconds(); }
        double ElapsedSince(date_type start) const { return static_cast<double>(Now() - start); }
    };

    using FRawTicks = TCounter<RawTicks>;
    // usingµ FThreadCycles = TCounter<ThreadCycles>;
    using FPerfCounter = TCounter<PerfCounter>;
    using FChronoTime = TCounter<ChronoTime>;

    using FCounter = FRawTicks;

    struct FTimer : FCounter {
        date_type StartedAt{ 0 };
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
            const double t = Elapsed;
            StartedAt = 0;
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
        :   _benchmark(benchmark)
        ,   _totalTime(0.0) {
            Assert(_benchmark.MinIterations >= FApproximateHistogram::MinSamples);
            Assert(_benchmark.MinIterations >= _benchmark.MaxIterations);
            _rnd.Seed(benchmark.RandomSeed);
        }

        FIterator begin();
        FIterator end();

        double TotalTime() const { return _totalTime; }
        FThreefy_4x32& Random() { return _rnd; }
        const FApproximateHistogram& Histogram() const { return _histogram; }

        void PauseTiming() { _timer.Stop(); }
        void ResumeTiming() {
            ClobberMemory(); // wait for pending memory read/writes before resuming the timer
            _timer.Start();
        }
        void ResetTiming() { // ignore elapsed time before, resume timer afterwards
            Assert_NoAssume(1 == _benchmark.BatchSize); // not supported for BatchSize > 1
            _timer.Reset();
            ResumeTiming();
        }

        void Finish() {
            // sort the reservoir to deduce the percentiles
            _histogram.Reservoir.Finalize();

            // log some progress : too heavy for MT, will saturate logging thread
            /*
            LOG(Benchmark, Info,
                L"{0:<18}: {1:10f4} < {2:10f4} < {3:10f4} => {4:10f4} µs  [{5:6} iterations] -> {6:f4}",
                _benchmark.Name,
                Low(), Mean(), High(),
                Median(),
                _histogram.NumSamples(),
                _histogram.SampleVariance());
                */
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

                const double t = (_timer.Reset() / (size_t(_benchmark.BatchSize) * _benchmark.InputDim));

                _totalTime += t;
                _histogram.AddSample(t, _rnd);
            }
        }

        NO_INLINE u32 RemainingIterations() const {
            return (
                (_histogram.NumSamples > _benchmark.MinIterations) && (
                (_histogram.NumSamples > _benchmark.MaxIterations) |
                (NearlyEquals(_histogram.Mean(), _histogram.WeightedMean(), _benchmark.MaxVarianceError)) )
                ? 0 : LoopCount() );
        }

    private:
        const FBenchmark& _benchmark;

        FTimer _timer;
        double _totalTime;
        FThreefy_4x32 _rnd;
        FApproximateHistogram _histogram;

        double VarianceError() const {
            return NearlyEquals(_histogram.Mean(), _histogram.WeightedMean(), _benchmark.MaxVarianceError);
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
        double Min, Q1, Median, Q3, Max;
        double Mode, Mean;
        double TotalTime;
    };

    template <typename _Benchmark, typename... _Args>
    static NO_INLINE FRun Run(_Benchmark& bm, const _Args&... args) {
        FState state{ bm };
        bm(state, args...);
        state.Finish();
        const auto& hist = state.Histogram();
        return FRun{
            hist.NumSamples,
            hist.Min(),
            hist.Q1(),
            hist.Median(),
            hist.Q3(),
            hist.Max(),
            hist.Mode(),
            hist.WeightedMean(),
            state.TotalTime() };
    }

public: // TTable<>
    template <typename... _Benchmarks>
    struct TTable {
        static constexpr size_t Dim = sizeof...(_Benchmarks);
        using FHeaders = TTuple<_Benchmarks...>;
        using FRow = TStaticArray<FRun, Dim>;

        struct FEntry {
            FStringView Name;
            FRow Row;

            CONSTEXPR explicit FEntry(const FStringView& name) NOEXCEPT : Name(name) {}
            CONSTEXPR explicit FEntry(const FStringView& name, FRow&& row) NOEXCEPT : Name(name), Row(std::move(row)) {}

            FEntry(const FEntry&) = default;
            FEntry& operator =(const FEntry&) = default;
            FEntry(FEntry&&) = default;
            FEntry& operator =(FEntry&&) = default;
        };

        using FEntries = VECTORINSITU(Diagnostic, FEntry, 8);

        FStringView Name;
        bool UseMultiThread{ true };

        FHeaders Headers;
        FEntries Entries;
        VECTOR(Benchmark, FTaskFunc) PendingRuns;

        size_t dim() const { return Dim; }

        TTable(const FStringView& name, _Benchmarks&&... headers)
        :   Name(name)
        ,   Headers(std::forward<_Benchmarks>(headers)...)
        {}

        ~TTable() {
            Flush();
        }

        void Add(const FStringView& name, FRow&& row) {
            Entries.emplace_back(name, std::move(row));
        }

        template <typename... _Args>
        void Run(const FStringView& name, const _Args&... args) {
            if (UseMultiThread)
                RunMT(name, args...);
            else
                RunST(name, args...);
        }

        template <typename _Lambda>
        void ForeachHeader(_Lambda foreach) const {
            ForeachTuple(Headers, foreach);
        }

        template <typename _Lambda>
        void ForeachEntry(_Lambda foreach) const {
            std::for_each(Entries.begin(), Entries.end(), foreach);
        }

        void Flush() {
            if (not PendingRuns.empty()) {
                Assert_NoAssume(UseMultiThread);
                PPE_LOG(Benchmark, Debug, "flushing {0} benchmark tasks...", PendingRuns.size());
                FTaskManager& threadPool =
#if 0           // high priority <=> full machine usage
                    FHighPriorityThreadPool::Get()
#else           // global <=> all cores but 1, leaves 2 threads idle for your leisure
                    FGlobalThreadPool::Get()
#endif
                    ;
                PPE_LOG_FLUSH();
                threadPool.RunAndWaitFor(PendingRuns.MakeView());
                PendingRuns.clear();
            }
        }

        template <typename... _Args>
        NO_INLINE void RunST(const FStringView& name, const _Args&... args) {
            PPE_LOG(Benchmark, Info, "running {0} benchmarks for <{1}> on a single thread...", Dim, name);

            Entries.emplace_back(name, MapTuple<FRun>(Headers, [&args...](const auto& bench) {
                return FBenchmark::Run(bench, args...);
            }));
        }

        template <typename... _Args>
        NO_INLINE void RunMT(const FStringView& name, const _Args&... args) {
            PPE_LOG(Benchmark, Info, "running {0} benchmarks for <{1}> on multiple threads...", Dim, name);

            size_t column = 0;
            const size_t row = Entries.size();

            Entries.emplace_back(name);

            ForeachHeader([&](const auto& bench) {
                PendingRuns.emplace_back(Meta::ForceInit/* remove capture size limit */,
                    [this, row, column, &bench, args...](ITaskContext&) {
                        FRun run = FBenchmark::Run(bench, args...);
                        Entries[row].Row[column] = std::move(run);
                    });
                column++;
            });

            Assert_NoAssume(Dim == column);
        }

    };

    template <typename... _Benchmarks>
    static TTable<_Benchmarks...> MakeTable(const FStringView& name, _Benchmarks&&... benchmarks) {
        return TTable<_Benchmarks...>(name, std::forward<_Benchmarks>(benchmarks)...);
    }

    template <typename... _Benchmarks>
    static void Flush(TTable<_Benchmarks...>& table) {
        table.Flush();
    }

    template <typename... _Benchmarks>
    static void FlushAndLog(TTable<_Benchmarks...>& table) {
        Flush(table);
        Log(table);
    }

public: // export table results
    template <typename... _Benchmarks, typename _Char>
    static void Txt(const TTable<_Benchmarks...>& table, TBasicTextWriter<_Char>& oss, bool detailed = false) {
        const auto originalFormat = oss .Format();
        DEFERRED {
            oss.SetFormat(originalFormat);
        };

        oss << FTextFormat::TruncateR;
        oss << STRING_LITERAL(_Char, "Benchmark table <") << table.Name << STRING_LITERAL(_Char, ">, units = ") << FCounter::Units() << STRING_LITERAL(_Char, " :") << Eol;

        constexpr u32 header = 20;
        constexpr u32 stride = 12;

        oss << STRING_LITERAL(_Char, '|')
            << FTextFormat::PadLeft(stride, STRING_LITERAL(_Char, ' '))
            << table.Name;

        table.ForeachHeader([&oss, stride](const auto& x) {
            oss << STRING_LITERAL(_Char, '|')
                << FTextFormat::PadLeft(stride, STRING_LITERAL(_Char, ' '))
                << x.Name;
        });

        oss << STRING_LITERAL(_Char, '|') << Eol;

        oss << Fmt::Repeat(STRING_LITERAL(_Char, '='), (stride+1) * table.dim() + header) << Eol;

        if (detailed) {
            table.ForeachEntry([&oss, stride, table_dim{table.dim()}](const auto& x) {
                oss << STRING_LITERAL(_Char, '|') << FTextFormat::PadLeft(stride, STRING_LITERAL(_Char, ' ')) << x.Name;

                for (const auto& c : x.Row)
                    oss << STRING_LITERAL(_Char, '|')
                        << FTextFormat::PadLeft(stride, STRING_LITERAL(_Char, ' '))
                        << FTextFormat::Float(FTextFormat::FixedFloat, 6)
                        << c.Q1 << STRING_LITERAL(_Char, '/')
                        << FTextFormat::PadLeft(stride, STRING_LITERAL(_Char, ' '))
                        << FTextFormat::Float(FTextFormat::FixedFloat, 6)
                        << c.Median << STRING_LITERAL(_Char, '/')
                        << FTextFormat::PadLeft(stride, STRING_LITERAL(_Char, ' '))
                        << FTextFormat::Float(FTextFormat::FixedFloat, 6)
                        << c.Q3;

                oss << STRING_LITERAL(_Char, '|') << Eol;
            });
        }
        else {
            table.ForeachEntry([&oss, stride, table_dim{table.dim()}](const auto& x) {
                oss << STRING_LITERAL(_Char, '|') << FTextFormat::PadLeft(stride, STRING_LITERAL(_Char, ' ')) << x.Name;

                for (const auto& c : x.Row)
                    oss << L'|'
                        << FTextFormat::PadLeft(stride, STRING_LITERAL(_Char, ' '))
                        << FTextFormat::Float(FTextFormat::FixedFloat, 6)
                        << c.Median;

                oss << L'|' << Eol;
            });
        }
    }

    template <typename... _Benchmarks>
    static void Log(const TTable<_Benchmarks...>& table) {
        Unused(table);
        PPE_LOG_DIRECT(Benchmark, Profiling, [&](FTextWriter& oss) {
            const bool detailed = (table.dim() < 5);
            Txt(table, oss, detailed);
        });
    }

    template <typename... _Benchmarks>
    static void Csv(const TTable<_Benchmarks...>& table, FTextWriter& oss) {
        Csv(table, oss, [](const FRun& run) {
            return run.Median;
        });
    }

    template <typename... _Benchmarks, typename _Projector>
    static void Csv(const TTable<_Benchmarks...>& table, FTextWriter& oss, const _Projector& projector) {
        oss << table.Name;

        table.ForeachHeader([&](const auto& x) {
            oss << ';' << x.Name;
        });

        oss << Eol;

        table.ForeachEntry([&](const auto& x) {
            oss << x.Name;
            for (const auto& c : x.Row)
                oss << ';' << projector(c);
            oss << Eol;
        });
    }

    template <typename... _Benchmarks>
    static void StackedBarCharts(const TTable<_Benchmarks...>& table, FTextWriter& oss) {
        oss << table.Name << ';' << "Stat";
        table.ForeachHeader([&oss](const auto& x) {
            oss << ';' << x.Name;
        });

        oss << Eol;

        table.ForeachEntry([&](const auto& x) {
            oss << x.Name << ";Min";
            for (const auto& c : x.Row)
                oss << ';' << c.Min;
            oss << Eol << x.Name << ";Q1";
            for (const auto& c : x.Row)
                oss << ';' << c.Q1;
            oss << Eol << x.Name << ";Median";
            for (const auto& c : x.Row)
                oss << ';' << (c.Median);
            oss << Eol << x.Name << ";Q3";
            for (const auto& c : x.Row)
                oss << ';' << (c.Q3);
            oss << Eol << x.Name << ";Max";
            for (const auto& c : x.Row)
                oss << ';' << (c.Max);
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

#undef USE_BENCHMARK_ASM_DONOTOPTIMIZE

#endif //!!(USE_PPE_FINAL_RELEASE)
