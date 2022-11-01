#include <numeric>

#include "stdafx.h"

#include "Allocator/SlabAllocator.h"
#include "Allocator/SlabHeap.h"
#include "Allocator/StlAllocator.h"
#include "Container/Appendable.h"
#include "Container/AssociativeVector.h"
#include "Container/BurstTrie.h"
#include "Container/FixedSizeHashTable.h"
#include "Container/FlatMap.h"
#include "Container/FlatSet.h"
#include "Container/HashTable.h"
#include "Container/MinMaxHeap.h"
#include "Container/SparseArray.h"
#include "Container/StringHashSet.h"
#include "Container/TupleVector.h"
#include "Container/Vector.h"
#include "Diagnostic/Benchmark.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformProcess.h"
#include "IO/BufferedStream.h"
#include "IO/FileStream.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "Maths/Maths.h"
#include "Maths/PrimeNumbers.h"
#include "Maths/RandomGenerator.h"
#include "Maths/Threefy.h"
#include "Memory/MemoryStream.h"
#include "Meta/PointerWFlags.h"
#include "Meta/Utility.h"
#include "Modular/Modular_fwd.h"
#include "Time/TimedScope.h"
#include "VirtualFileSystem.h"

#include <numeric>
#include <random>
#include <unordered_set>

#define PPE_RUN_BENCHMARK_POOLSIZE          (2000) // avoid cache coherency
#define PPE_RUN_EXHAUSTIVE_BENCHMARKS       (0) // %_NOCOMMIT%
#define PPE_RUN_BENCHMARK_ALLTESTS          (0) // %_NOCOMMIT%
#define PPE_RUN_BENCHMARK_ONE_CONTAINER     (0) // %_NOCOMMIT%
#define PPE_RUN_BENCHMARK_MULTITHREADED     (1) // %_NOCOMMIT%
#define PPE_DONT_USE_STD_UNORDEREDSET       (1) // %_NOCOMMIT%
#define USE_PPE_CONTAINERS_LONGRUN          (0) // %_NOCOMMIT%
#define USE_PPE_CONTAINERS_MEMOIZER         (0) // %_NOCOMMIT%
#define USE_PPE_CONTAINERS_DEBUGGING        (0) // %_NOCOMMIT%
#define USE_PPE_CONTAINERS_FASTRANDOM       (0) // %_NOCOMMIT%
#define USE_PPE_CONTAINERS_STRING           (0) // %_NOCOMMIT%

static_assert(PPE_RUN_BENCHMARK_POOLSIZE > 0, "must have at least one sample");

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wunused-variable"
#endif

namespace PPE {
LOG_CATEGORY(, Test_Containers)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template class TAssociativeVector<FString, int>;
template class TAssociativeVector<
    u64, FString,
    Meta::TEqualTo<u64>,
    SPARSEARRAY_INSITU(Container, TPair< u64 COMMA FString >)
>;
template class TFlatMap<FString, int>;
template class TFlatSet<FString>;
//template class TBasicHashTable< details::THashMapTraits_<FString, int>, Meta::THash<FString>, Meta::TEqualTo<FString>, ALLOCATOR(Container, TPair<FString COMMA int>)>; #TODO : after migrating default hash map
//template class TBasicHashTable< details::THashSetTraits_<FString>, Meta::THash<FString>, Meta::TEqualTo<FString>, ALLOCATOR(Container, FString)>; #TODO : after migrating default hash map
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Containers/BulkTrie.h"
#include "Containers/CompactHashSet.h"
#include "Containers/DenseHashSet.h"
#include "Containers/DenseHashSet2.h"
#include "Containers/DenseHashSet3.h"
#include "Containers/HopscotchHashSet.h"
#include "Containers/HopscotchHashSet2.h"
#include "Containers/SimdHashSet.h"
#include "Containers/SSEHashSet2.h"
#include "Containers/SSEHashSet3.h"
#include "Containers/SSEHashSet4.h"
#include "Containers/SSEHashSet5.h"
#include "Containers/SSEHashSet6.h"

namespace PPE {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
namespace BenchmarkContainers {
template <typename T>
struct TSamplePool {
    struct FSampleData {
        u32 SeriesIndex;
        VECTOR(Benchmark, T) Insert;
        VECTOR(Benchmark, T) Unknown;
        VECTOR(Benchmark, T) Search;
        VECTOR(Benchmark, T) Erase;
        VECTOR(Benchmark, T) Dense;
        VECTOR(Benchmark, T) Sparse;

        template <typename _Container>
        void FillDense(_Container& c) const {
            c.reserve(Insert.size());
#if (!USE_PPE_CONTAINERS_DEBUGGING)
            for (const auto& it : Insert) {
                c.insert(it);
                Assert_NoAssume(c.find(it) != c.end());
            }
#else
            forrange(i, 0, Insert.size()) {
                c.insert(Insert[i]);
                Assert_NoAssume(c.size() == i + 1);

                for (u32 j = 0; j <= i; ++j)
                    AssertRelease(c.find(Insert[j]) != c.end());
            }
#endif
            Assert_NoAssume(c.size() == Insert.size());
        }

        template <typename _Container>
        void FillSparse(_Container& c) const {
            FillDense(c);

            for (const auto& it : Sparse)
                Verify(c.erase(it));

            Assert_NoAssume(c.size() == Insert.size() - Sparse.size());
        }
    };

    size_t NumSamples;
    VECTOR(Benchmark, u32) RandomOrder;
    VECTOR(Benchmark, FSampleData) Series;

    TSamplePool() : NumSamples(0) {}

    TSamplePool(const TSamplePool&) = delete;
    TSamplePool& operator =(const TSamplePool&) = delete;

    struct FSeriesIterator {
        const TSamplePool& Generator;
        u32 Index;

        explicit FSeriesIterator(const TSamplePool& generator)
            : Generator(generator)
            , Index(0)
        {}

        const FSampleData& Next() {
            const size_t r = Index++ % Generator.RandomOrder.size();
            return Generator.Series[Generator.RandomOrder[r]];
        }
    };

    FSeriesIterator MakeSeries() const { return FSeriesIterator(*this); }

    template <typename _Container, typename _Lambda>
    void MakeDenseTables(const _Container& archetype, _Lambda&& lambda) const {
        STACKLOCAL_STACK(_Container, tables, Series.size());

        for (const FSampleData& series : Series) {
            tables.Push(archetype);
            series.FillDense(*tables.Peek());
        }

        lambda(tables.MakeView());
    }

    template <typename _Container, typename _Lambda>
    void MakeSparseTables(const _Container& archetype, _Lambda&& lambda) const {
        STACKLOCAL_STACK(_Container, tables, Series.size());

        for (const FSampleData& series : Series) {
            tables.Push(archetype);
            series.FillSparse(*tables.Peek());
        }

        lambda(tables.MakeView());
    }

    template <typename _Container, typename _Lambda>
    void MakeTablesN(const _Container& archetype, u32 n, _Lambda&& lambda) const {
        STACKLOCAL_STACK(_Container, tables, Series.size());

        for (const FSampleData& series : Series) {
            Assert_NoAssume(series.Insert.size() >= n);

            tables.Push(archetype);
            auto& h = *tables.Peek();

            h.reserve(n);
            for (const auto& it : series.Insert.MakeConstView().CutBefore(n))
                h.insert(it);
        }

        lambda(tables.MakeView());
    }

    template <typename _Rnd, typename _Generator>
    void Generate(size_t series, size_t numSamples, u32 jitter, _Rnd& rnd, const _Generator& generatorArchetype) {
        _Generator generator{ generatorArchetype }; // copy the generator before generating samples

        NumSamples = numSamples;

        VECTOR(Benchmark, T) samples;

        const size_t totalSamples = (numSamples * 4); // generate 4x, will be shuffled

        // one pool of generated samples for each series
        samples.clear();
        samples.reserve(totalSamples);
        // generate exactly N unique samples
        forrange(i, 0, totalSamples)
            samples.emplace_back(generator(rnd));

        // one random sequence to pick each series randomly
        RandomOrder.clear();
        RandomOrder.reserve(numSamples);
        forrange(i, 0, numSamples)
            RandomOrder.emplace_back(u32(i % series));
        //std::shuffle(RandomOrder.begin(), RandomOrder.end(), rnd);
        rnd.Shuffle(RandomOrder.MakeView());

        // generate random shuffles for each series
        VECTOR(Benchmark, u32) shuffled;
        shuffled.reserve(totalSamples);
        forrange(i, 0, totalSamples)
            shuffled.emplace_back(u32(i));

        Series.clear();
        Series.reserve(series);
        forrange(i, 0, series) {
            // jitter can be used to vary the size of each series
            const size_t jitteredSamples = jitter ? rnd(
                checked_cast<u32>(numSamples - jitter),
                checked_cast<u32>(numSamples + jitter) ) : numSamples;
            Assert_NoAssume(jitteredSamples > 0);

            Series.emplace_back_AssumeNoGrow();
            FSampleData& s = Series.back();
            s.SeriesIndex = checked_cast<u32>(i);
            {
                //std::shuffle(shuffled.begin(), shuffled.end(), rnd);
                rnd.Shuffle(shuffled.MakeView());
                TMemoryView<const u32> pool = shuffled.MakeConstView();

                s.Insert.reserve(jitteredSamples);
                for (u32 sp : pool.Eat(jitteredSamples))
                    s.Insert.push_back(samples[sp]);

                s.Unknown.reserve(jitteredSamples);
                for (u32 sp : pool.Eat(jitteredSamples))
                    s.Unknown.push_back(samples[sp]);
            }

            constexpr size_t sparse_factor = 70;

            s.Search.assign(s.Insert);
            //std::shuffle(s.Search.begin(), s.Search.end(), rnd);
            rnd.Shuffle(s.Search.MakeView());

            s.Erase.assign(s.Search);
            //std::shuffle(s.Erase.begin(), s.Erase.end(), rnd);
            rnd.Shuffle(s.Erase.MakeView());

            s.Sparse.assign(s.Erase
                .MakeConstView()
                .CutBeforeConst((sparse_factor * s.Erase.size()) / 100) );
            //std::shuffle(s.Sparse.begin(), s.Sparse.end(), rnd);
            rnd.Shuffle(s.Sparse.MakeView());

            s.Dense.assign(s.Erase
                .MakeConstView()
                .CutStartingAt(s.Sparse.size()) );
            //std::shuffle(s.Dense.begin(), s.Dense.end(), rnd);
            rnd.Shuffle(s.Dense.MakeView());
        }
    }

    template <typename _Generator>
    static auto MakeUniqueGenerator(const _Generator& generator) {
        return MakeUniqueGenerator(generator, THashSet<T>{});
    }

    template <typename _Generator, typename _Set>
    struct TUniqueSampleGenerator {
        _Generator Generator;
        _Set Unique;
        TUniqueSampleGenerator(const _Generator& generator, _Set&& unique)
            : Generator(generator)
            , Unique(std::move(unique))
        {}
        template <typename _Rnd>
        auto operator ()(_Rnd& rnd) {
            using sample_type = decltype(Generator(rnd));
            sample_type sample;
            for (;;) {
                sample = Generator(rnd);
                if (not Unique.insert_ReturnIfExists(sample))
                    break;
            }
            return sample;
        }
    };

    template <typename _Generator, typename _Set>
    static auto MakeUniqueGenerator(const _Generator& generator, _Set&& unique) {
        return TUniqueSampleGenerator<_Generator, _Set>{ generator, std::move(unique) };
    }
};
class FContainerBenchmark : public FBenchmark {
public:
    FContainerBenchmark(const FStringView& name)
    :   FBenchmark{ name } {
#if !USE_PPE_CONTAINERS_LONGRUN
        MaxIterations = Min(MaxIterations, 1000000_u32);
        ONLY_IF_ASSERT(MaxIterations = Min(MinIterations, MaxIterations));
#else
        // should run a fixed number of iterations for volume comparisons
#   if USE_PPE_DEBUG
        MinIterations = MaxIterations = 10000_u32;
#   else
        MinIterations = MaxIterations = 1000000_u32;
#   endif
        MaxVarianceError = 1e-9f;
#endif
    }
};
class construct_noreserve_t : public FContainerBenchmark {
public:
    construct_noreserve_t() : FContainerBenchmark{ "ctor_cold" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        auto series = pool->MakeSeries();
        for (auto _ : state) {
            const auto& s = series.Next();
            auto c{ archetype };

            state.ResetTiming();

            for (const auto& it : s.Insert)
                Verify(c.insert(it).second);

            FBenchmark::DoNotOptimize(c);
        }
    }
};
class construct_reserve_t : public FContainerBenchmark {
public:
    construct_reserve_t() : FContainerBenchmark{ "ctor_warm" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        auto series = pool->MakeSeries();
        for (auto _ : state) {
            const auto& s = series.Next();
            auto c{ archetype };
            c.reserve(s.Insert.size());

            state.ResetTiming();

            for (const auto& it : s.Insert)
                Verify(c.insert(it).second);

            FBenchmark::DoNotOptimize(c);
        }
    }
};
class copy_empty_t : public FContainerBenchmark {
public:
    copy_empty_t() : FContainerBenchmark{ "copy_pty" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto c{ archetype };
            auto series = pool->MakeSeries();

            for (_Container& table : tables)
                table.clear(); // test traversal times for grown empty tables

            for (auto _ : state) {
                const auto& s = series.Next();
                c.clear();

                state.ResetTiming();

                c = tables[s.SeriesIndex];
                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class copy_dense_t : public FContainerBenchmark {
public:
    copy_dense_t() : FContainerBenchmark{ "copy_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();

                state.ResetTiming();

                auto c{ tables[s.SeriesIndex] };
                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class copy_sparse_t : public FContainerBenchmark {
public:
    copy_sparse_t() : FContainerBenchmark{ "copy_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();

                state.ResetTiming();

                auto c{ tables[s.SeriesIndex] };
                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class insert_dense_t : public FContainerBenchmark {
public:
    insert_dense_t() : FContainerBenchmark{ "insert_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                state.PauseTiming();

                const auto& s = series.Next();
                auto c{ tables[s.SeriesIndex] };

                //const auto& item = state.Random().RandomElement(s.Unknown.MakeView());
                const size_t N = ((c.size() + 9) / 10);
                const auto insert = s.Unknown.MakeConstView().SubRange(
                    state.Random()(0, checked_cast<u32>(s.Unknown.size() - N)), N);

                state.ResumeTiming();

#if 0
                auto it = c.insert(item);
                Assert_NoAssume(it.second);

                FBenchmark::DoNotOptimize(*it.first);
#else
                for (const auto& item : insert) {
                    auto it = c.insert(item);
                    Assert_NoAssume(it.second);
                    FBenchmark::DoNotOptimize(*it.first);
                }

                FBenchmark::DoNotOptimize(c);
#endif
            }
        });
    }
};
class insert_sparse_t : public FContainerBenchmark {
public:
    insert_sparse_t() : FContainerBenchmark{ "insert_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                state.PauseTiming();

                const auto& s = series.Next();
                auto c{ tables[s.SeriesIndex] };

                //const auto& item = state.Random().RandomElement(s.Unknown.MakeView());
                const size_t N = ((c.size() + 9) / 10);
                const auto insert = s.Unknown.MakeConstView().SubRange(
                    state.Random()(0, checked_cast<u32>(s.Unknown.size() - N)), N);

                state.ResumeTiming();

#if 0
                auto it = c.insert(item);
                Assert_NoAssume(it.second);

                FBenchmark::DoNotOptimize(*it.first);
#else
                for (const auto& item : insert) {
                    auto it = c.insert(item);
                    Assert_NoAssume(it.second);
                    FBenchmark::DoNotOptimize(*it.first);
                }

                FBenchmark::DoNotOptimize(c);
#endif
            }
        });
    }
};
class iterate_dense_t : public FContainerBenchmark {
public:
    iterate_dense_t() : FContainerBenchmark{ "iter_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                const auto& c = tables[s.SeriesIndex];

                state.ResetTiming();

                size_t n = 0;
                for (const auto& it : c) {
                    Assert_NoAssume(n < c.size());
                    FBenchmark::DoNotOptimize(it);
                    n++;
                }

                Assert_NoAssume(c.size() == n);
                FBenchmark::DoNotOptimize(n);
            }
        });
    }
};
class iterate_sparse_t : public FContainerBenchmark {
public:
    iterate_sparse_t() : FContainerBenchmark{ "iter_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                const auto& c = tables[s.SeriesIndex];

                state.ResetTiming();

                size_t n = 0;
                for (const auto& it : c) {
                    FBenchmark::DoNotOptimize(it);
                    n++;
                }

                Assert_NoAssume(c.size() == n);
                FBenchmark::DoNotOptimize(n);
            }
        });
    }
};
class find_dense_pos_t : public FContainerBenchmark {
public:
    find_dense_pos_t() : FContainerBenchmark{ "find_dns_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                const auto& c = tables[s.SeriesIndex];

                state.ResetTiming();

                uintptr_t hit = 0;
                for (const auto& it : s.Search) {
                    auto jt = c.find(it);
                    FContainerBenchmark::DoNotOptimize(jt);
                    hit += uintptr_t(&*jt);
                }

                FBenchmark::DoNotOptimize(hit);
            }
        });
    }
};
class find_dense_neg_t : public FContainerBenchmark {
public:
    find_dense_neg_t() : FContainerBenchmark{ "find_dns_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                const auto& c = tables[s.SeriesIndex];

                state.ResetTiming();

                u32 hit = 0;
                const auto cend = c.end();
                for (const auto& it : s.Unknown) {
                    auto jt = c.find(it);
                    FBenchmark::DoNotOptimize(jt);
                    if (jt != cend) hit++;
                }

                Assert_NoAssume(0 == hit);
                FBenchmark::DoNotOptimize(hit);
            }
        });
    }
};
class find_sparse_pos_t : public FContainerBenchmark {
public:
    find_sparse_pos_t() : FContainerBenchmark{ "find_spr_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                const auto& c = tables[s.SeriesIndex];

                state.ResetTiming();

                ONLY_IF_ASSERT(size_t n = 0);
                uintptr_t h = 0;
                for (const auto& it : s.Dense) {
                    auto jt = c.find(it);
                    FContainerBenchmark::DoNotOptimize(jt);
                    h += uintptr_t(&*jt);
                    ONLY_IF_ASSERT(n++);
                }

                Assert_NoAssume(s.Dense.size() == n);
                FBenchmark::DoNotOptimize(h);
            }
        });
    }
};
class find_sparse_neg_t : public FContainerBenchmark {
public:
    find_sparse_neg_t() : FContainerBenchmark{ "find_spr_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                const auto& c = tables[s.SeriesIndex];

                state.ResetTiming();

                u32 hit = 0;
                const auto cend = c.end();
                for (const auto& it : s.Unknown) {
                    auto jt = c.find(it);
                    FBenchmark::DoNotOptimize(jt);
                    if (jt != cend) hit++;
                }

                Assert_NoAssume(0 == hit);
                FBenchmark::DoNotOptimize(hit);
            }
        });
    }
};
class erase_dense_pos_t : public FContainerBenchmark {
public:
    erase_dense_pos_t() : FContainerBenchmark{ "erase_dns_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                auto c{ tables[s.SeriesIndex] };

                state.ResetTiming();

                for (const auto& it : s.Search)
                    Verify(c.erase(it));

                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class erase_dense_neg_t : public FContainerBenchmark {
public:
    erase_dense_neg_t() : FContainerBenchmark{ "erase_dns_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                auto& c = tables[s.SeriesIndex];

                state.ResetTiming();

                for (const auto& it : s.Unknown)
                    Verify(not c.erase(it));

                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class erase_sparse_pos_t : public FContainerBenchmark {
public:
    erase_sparse_pos_t() : FContainerBenchmark{ "erase_spr_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                auto c{ tables[s.SeriesIndex] };

                state.ResetTiming();

                for (const auto& it : s.Dense)
                    Verify(c.erase(it));

                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class erase_sparse_neg_t : public FContainerBenchmark {
public:
    erase_sparse_neg_t() : FContainerBenchmark{ "erase_spr_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                auto& c = tables[s.SeriesIndex];

                state.ResetTiming();

                for (const auto& it : s.Unknown)
                    Verify(not c.erase(it));

                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class find_erase_dns_t : public FContainerBenchmark {
public:
    find_erase_dns_t() : FContainerBenchmark{ "find_erase_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                auto c{ tables[s.SeriesIndex] };

                const size_t ns = s.Sparse.size();
                const size_t nu = s.Unknown.size();
                const size_t nn = Min(nu, ns);

                state.ResetTiming();

                forrange(i, 0, nn) {
                    Verify(c.insert(s.Unknown[i]).second);
                    Verify(c.erase(s.Sparse[i]));
                }

                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class find_erase_spr_t : public FContainerBenchmark {
public:
    find_erase_spr_t() : FContainerBenchmark{ "find_erase_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                auto c{ tables[s.SeriesIndex] };

                const size_t nd = s.Dense.size();
                const size_t nu = s.Unknown.size();
                const size_t nn = Min(nu, nd);

                state.ResetTiming();

                forrange(i, 0, nn) {
                    Verify(c.insert(s.Unknown[i]).second);
                    Verify(c.erase(s.Dense[i]));
                }

                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class clear_dense_t : public FContainerBenchmark {
public:
    clear_dense_t() : FContainerBenchmark{ "clear_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeDenseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                auto c{ tables[s.SeriesIndex] };

                state.ResetTiming();

                c.clear();

                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
class clear_sparse_t : public FContainerBenchmark {
public:
    clear_sparse_t() : FContainerBenchmark{ "clear_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeSparseTables(archetype, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                auto c{ tables[s.SeriesIndex] };

                state.ResetTiming();

                c.clear();

                FBenchmark::DoNotOptimize(c);
            }
        });
    }
};
} //!namespace BenchmarkContainers
//----------------------------------------------------------------------------
template <typename T, typename _Generator, typename _Containers>
NO_INLINE static void Benchmark_Containers_Exhaustive_(
    const FStringView& name, size_t dim,
    _Generator&& generator,
    _Containers&& tests ) {
    // prepare input data

    LOG(Test_Containers, Emphasis, L"Running benchmark <{0}> with {1} tests :", name, dim);

#if 0
    // mt19937 has better distribution than FRandomGenerator for generating benchmark data
    std::random_device rdevice;
    std::mt19937 rand{ rdevice() };
    rand.seed(0x9025u); // fixed seed for repro
#elif USE_PPE_CONTAINERS_FASTRANDOM
    // FRandomGeneratoor, poor statistical properties
    FRandomGenerator rand;
    rand.Reset(0x9025u);
#else
    // use new Threefy generator
    FThreefy_4x32 rand;
    rand.Seed(0x9025u);
#endif

    using namespace BenchmarkContainers;

    // prepare PPE_RUN_BENCHMARK_POOLSIZE different samples with random distribution and 2% jitter

    const u32 jitter = checked_cast<u32>((dim * 2) / 100);

    TSamplePool<T> pool;
    pool.Generate(PPE_RUN_BENCHMARK_POOLSIZE, dim, jitter, rand, generator);

    // prepare benchmark table

    auto bm = FBenchmark::MakeTable(
        name,
#if (PPE_RUN_BENCHMARK_ALLTESTS || PPE_RUN_EXHAUSTIVE_BENCHMARKS)
        construct_noreserve_t{},
        construct_reserve_t{},
        copy_empty_t{},
        copy_dense_t{},
        copy_sparse_t{},
        insert_dense_t{},
        insert_sparse_t{},
        iterate_dense_t{},
        iterate_sparse_t{},
        find_dense_pos_t{},
        find_dense_neg_t{},
        find_sparse_pos_t{},
        find_sparse_neg_t{},
        erase_dense_pos_t{},
        erase_dense_neg_t{},
        erase_sparse_pos_t{},
        erase_sparse_neg_t{},
        find_erase_dns_t{},
        find_erase_spr_t{},
        clear_dense_t{},
        clear_sparse_t{}
#else
        insert_dense_t{},
        insert_sparse_t{},
        erase_dense_pos_t{},
        erase_dense_neg_t{},
        erase_sparse_pos_t{},
        erase_sparse_neg_t{}
#endif
    );

    bm.UseMultiThread = !!(PPE_RUN_BENCHMARK_MULTITHREADED);

    tests(bm, &pool);

    FBenchmark::FlushAndLog(bm);

    {
        const FFilename fname{ StringFormat(L"Saved:/Benchmark/{0}/Containers/{1}.csv",
            MakeStringView(WSTRINGIZE(BUILD_FAMILY)), name) };

        FStringBuilder sb;
        FBenchmark::StackedBarCharts(bm, sb);
        VFS_WriteAll(fname, sb.Written().RawView(), EAccessPolicy::Truncate_Binary | EAccessPolicy::Roll);
    }
}
//----------------------------------------------------------------------------
namespace BenchmarkContainers {
class findspeed_dense_pos_t : public FContainerBenchmark {
public:
    findspeed_dense_pos_t(size_t n, const FStringView& name)
    :   FContainerBenchmark{ name } {
        InputDim = checked_cast<u32>(n);
    }
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeTablesN(archetype, InputDim, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                const auto& c = tables[s.SeriesIndex];

                const TMemoryView<const T> samples = s.Search.MakeConstView().CutBefore(InputDim);

                state.ResetTiming();

                for (const auto& it : samples)
                    FBenchmark::DoNotOptimize(c.find(it));
            }
        });
    }
};
class findspeed_dense_neg_t : public FContainerBenchmark {
public:
    findspeed_dense_neg_t(size_t n, const FStringView& name)
    :   FContainerBenchmark{ name } {
        InputDim = checked_cast<u32>(n);
    }
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TSamplePool<T>* pool) const {
        pool->MakeTablesN(archetype, InputDim, [&](const TMemoryView<_Container>& tables) {
            auto series = pool->MakeSeries();

            for (auto _ : state) {
                const auto& s = series.Next();
                const auto& c = tables[s.SeriesIndex];

                const TMemoryView<const T> samples = s.Unknown.MakeConstView().CutBefore(InputDim);

                state.ResetTiming();

                for (const auto& it : samples)
                    FBenchmark::DoNotOptimize(c.find(it));
            }
        });
    }
};
} //!namespace BenchmarkContainers
template <typename _Test, typename T, typename _Containers>
static void Benchmark_Containers_FindSpeed_Impl_(
    const FStringView& name,
    _Containers& tests,
    const BenchmarkContainers::TSamplePool<T>& pool ) {
    auto bm = FBenchmark::MakeTable(
        name,
        _Test{ 4, "4" },
        _Test{ 6, "6" },
        _Test{ 9, "9" },
        _Test{ 12, "12" },
        _Test{ 16, "16" },
        _Test{ 20, "20" },
        _Test{ 25, "25" },
        _Test{ 30, "30" },
        _Test{ 36, "36" },
        _Test{ 42, "42" },
        _Test{ 49, "49" },
        _Test{ 56, "56" },
        _Test{ 64, "64" },
        _Test{ 72, "72" },
        _Test{ 81, "81" },
        _Test{ 90, "90" },
        _Test{ 100, "100" },
        _Test{ 110, "110" },
        _Test{ 121, "121" },
        _Test{ 132, "132" },
        _Test{ 144, "144" },
        _Test{ 156, "156" },
        _Test{ 169, "169" },
        _Test{ 182, "182" },
        _Test{ 196, "196" }
#if !USE_PPE_ASSERT
        ,
        _Test{ 210, "210" },
        _Test{ 225, "225" },
        _Test{ 240, "240" },
        _Test{ 256, "256" },
        _Test{ 272, "272" },
        _Test{ 289, "289" },
        _Test{ 306, "306" },
        _Test{ 324, "324" },
        _Test{ 342, "342" },
        _Test{ 361, "361" },
        _Test{ 380, "380" },
        _Test{ 400, "400" },
        _Test{ 420, "420" },
        _Test{ 441, "441" },
        _Test{ 462, "462" },
        _Test{ 484, "484" }
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS
        ,
        _Test{ 506, "506" },
        _Test{ 529, "529" },
        _Test{ 552, "552" },
        _Test{ 576, "576" },
        _Test{ 600, "600" },
        _Test{ 625, "625" },
        _Test{ 650, "650" },
        _Test{ 676, "676" },
        _Test{ 702, "702" },
        _Test{ 729, "729" },
        _Test{ 756, "756" },
        _Test{ 784, "784" },
        _Test{ 812, "812" },
        _Test{ 841, "841" },
        _Test{ 870, "870" },
        _Test{ 900, "900" },
        _Test{ 930, "930" },
        _Test{ 961, "961" },
        _Test{ 992, "992" },
        _Test{ 1024, "1024" },
        _Test{ 1056, "1056" },
        _Test{ 1089, "1089" },
        _Test{ 1122, "1122" }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
#endif //!USE_PPE_ASSERT
    );

    bm.UseMultiThread = !!(PPE_RUN_BENCHMARK_MULTITHREADED);

    tests(bm, &pool);

    FBenchmark::FlushAndLog(bm);

#if 1
    {
        const FFilename fname{ StringFormat(L"Saved:/Benchmark/{0}/Containers/{1}.csv",
            MakeStringView(WSTRINGIZE(BUILD_FAMILY)), name) };

        FStringBuilder sb;
        FBenchmark::StackedBarCharts(bm, sb);
        VFS_WriteAll(fname, sb.Written().RawView(), EAccessPolicy::Truncate_Binary | EAccessPolicy::Roll);
    }
#else
    auto graph = [&](const FWStringView& feature, auto projector) {
        const FFilename fname{ StringFormat(L"Saved:/Benchmark/{0}/Containers/{1}_{2}.csv",
            MakeStringView(WSTRINGIZE(BUILD_FAMILY)), name, feature) };

        FStringBuilder sb;
        FBenchmark::Csv(bm, sb, projector);
        VFS_WriteAll(fname, sb.Written().RawView(), EAccessPolicy::Truncate_Binary | EAccessPolicy::Roll);
    };

    graph(L"mean", [](const auto& run) { return run.Mean; });
    graph(L"median", [](const auto& run) { return run.Median; });
    graph(L"mode", [](const auto& run) { return run.Mode; });
#endif
}
template <typename T, typename _Generator, typename _Containers>
NO_INLINE static void Benchmark_Containers_FindSpeed_(const FStringView& name, _Generator&& generator, _Containers&& tests) {
    LOG(Test_Containers, Emphasis, L"Running find speed benchmarks <{0}> :", name);

#if 0
    // mt19937 has better distribution than FRandomGenerator for generating benchmark data
    std::random_device rdevice;
    std::mt19937 rand{ rdevice() };
    rand.seed(0x565Fu); // fixed seed for repro
#elif USE_PPE_CONTAINERS_FASTRANDOM
    // use new Threefy generator
    FRandomGenerator rand;
    rand.Reset(0x565Fu);
#else
    // use new Threefy generator
    FThreefy_4x32 rand;
    rand.Seed(0x565Fu);
#endif

    using namespace BenchmarkContainers;

    // prepare 128 different sample series with random distribution

    TSamplePool<T> pool;
    pool.Generate(PPE_RUN_BENCHMARK_POOLSIZE, 2048, 0/* no jitter */, rand, generator);

    Benchmark_Containers_FindSpeed_Impl_<findspeed_dense_pos_t>(FString(name) + "_pos", tests, pool);
    Benchmark_Containers_FindSpeed_Impl_<findspeed_dense_neg_t>(FString(name) + "_neg", tests, pool);
}
//----------------------------------------------------------------------------
template <typename T, typename _Generator>
NO_INLINE static void Test_PODSet_(const FString& name, const _Generator& samples) {
    auto containers_large = [](auto& bm, const auto* input) {
        /*{ // very buggy
            typedef TCompactHashSet<T> hashtable_type;

            hashtable_type set;
            bm.Run("TCompactHashSet", set, input);
        }*/
#if 0//!PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            TDenseHashSet2<T> Dense2;
            bm.Run("Dense2", Dense2, input);
        }
#   if USE_PPE_CONTAINERS_MEMOIZER
        {
            TDenseHashSet2<THashMemoizer<T>> Dense2_M;
            bm.Run("Dense2_M", Dense2_M, input);
        }
#   endif
#endif
#if 0 && !PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            TDenseHashSet3<T> Dense3;
            bm.Run("Dense3", Dense3, input);
        }
#   if USE_PPE_CONTAINERS_MEMOIZER
        {
            TDenseHashSet3<THashMemoizer<T>> Dense3;
            bm.Run("Dense3_M", Dense3, input);
        }
#   endif
#endif
#if 0//!PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            THopscotchHashSet<T> Hopscotch;
            bm.Run("Hopscotch", Hopscotch, input);
        }
#   if USE_PPE_CONTAINERS_MEMOIZER
        {
            THopscotchHashSet<THashMemoizer<T>> Hopscotch_M;
            bm.Run("Hopscotch_M", Hopscotch_M, input);
        }
#   endif
#endif
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            THopscotchHashSet2<T> Hopscotch2;
            bm.Run("Hopscotch2", Hopscotch2, input);
        }
#endif
#if !PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            THashSet<T> HashSet;
            bm.Run("HashSet", HashSet, input);
        }
#endif
#if !PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            THashSet<T, Meta::TCRC32<T> > HashSet;
            bm.Run("HashSet_CRC32", HashSet, input);
        }
#endif
#if 0//!PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            TSSEHashSet<T> set;
            bm.Run("SSEHashSet", set, input);
        }
#endif
#if 1//!PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            TSSEHashSet2<T> set;
            bm.Run("SSEHashSet2", set, input);
        }
#endif
#if 1//!PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            TSSEHashSet2<T, Meta::TCRC32<T> > set;
            bm.Run("SSEHashSet2_CRC32", set, input);
    }
#endif
#if 0//USE_PPE_AVX2//!PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            TSSEHashSet3<T> set;
            bm.Run("SSEHashSet3", set, input);
        }
#endif
#if !PPE_RUN_BENCHMARK_ONE_CONTAINER
        {
            TSSEHashSet4<T> set;
            bm.Run("SSEHashSet4", set, input);
        }
#endif
#if 0//PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            TSSEHashSet5<T> set;
            bm.Run("SSEHashSet5", set, input);
        }
#endif
#if 0//PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            TSSEHashSet5<T, Meta::TCRC32<T> > set;
            bm.Run("SSEHashSet5_CRC32", set, input);
        }
#endif
#if 0//PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            TSSEHashSet6<T> set;
            bm.Run("SSEHashSet6", set, input);
        }
#endif
#if 0//PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            TSSEHashSet6<T, Meta::TCRC32<T>> set;
            bm.Run("SSEHashSet6_CRC32", set, input);
        }
#endif
#if !PPE_RUN_BENCHMARK_ONE_CONTAINER && !PPE_DONT_USE_STD_UNORDEREDSET
        {
            std::unordered_set<
                T, Meta::THash<T>, Meta::TEqualTo<T>,
                TStlAllocator< T, ALLOCATOR(Container) >
            >   unordered_set;
            bm.Run("unordered_set", unordered_set, input);
        }
#endif
    };

    auto containers_all = [&](auto& bm, const auto* input) {
#if 0 //!PPE_RUN_BENCHMARK_ONE_CONTAINER && PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            typedef TVector<T> vector_type;

            vector_type v;

            struct FAdapter_ {
                vector_type v;
                size_t size() const { return v.size(); }
                auto begin() const { return v.begin(); }
                auto end() const { return v.end(); }
                void insert(T i) { v.push_back(i); }
                auto find(T i) const { return std::find(v.begin(), v.end(), i); }
                void reserve(size_t n) { v.reserve(n); }
                bool erase(T i) {
                    auto it = find(i);
                    if (v.end() == it)
                        return false;
                    v.erase_DontPreserveOrder(it);
                    return true;
                }
                void clear() { v.clear(); }
            };

            FAdapter_ set;
            bm.Run("Vector", set, input);
        }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
#if 0 //!PPE_RUN_BENCHMARK_ONE_CONTAINER && PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            typedef TSparseArray<T> sparse_array_t;

            sparse_array_t v;

            struct FAdapter_ {
                sparse_array_t a;
                size_t size() const { return a.size(); }
                auto begin() const { return a.begin(); }
                auto end() const { return a.end(); }
                void insert(T i) { a.Emplace(i); }
                auto find(T i) const { return std::find(a.begin(), a.end(), i); }
                void reserve(size_t n) { a.Reserve(n); }
                bool erase(T i) {
                    auto it = find(i);
                    if (a.end() == it)
                        return false;
                    a.Remove(it);
                    return true;
                }
                void clear() { a.Clear(); }
            };

            FAdapter_ set;
            bm.Run("SparseArray", set, input);
        }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
#if 0 //!PPE_RUN_BENCHMARK_ONE_CONTAINER && PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            TFlatSet<T> set;
            bm.Run("FlatSet", set, input);
        }
        {
            TFixedSizeHashSet<T, 2048> set;
            bm.Run("FixedSize", set, input);
        }
#endif
        containers_large(bm, input);
    };

    using namespace BenchmarkContainers;

    auto generator = TSamplePool<T>::MakeUniqueGenerator(samples);

#if 1
    Benchmark_Containers_Exhaustive_<T>(name + "_20", 20, generator, containers_all);
#   if PPE_RUN_EXHAUSTIVE_BENCHMARKS || PPE_RUN_BENCHMARK_ONE_CONTAINER
    Benchmark_Containers_Exhaustive_<T>(name + "_30", 30, generator, containers_all);
    Benchmark_Containers_Exhaustive_<T>(name + "_40", 40, generator, containers_all);
#   endif
    Benchmark_Containers_Exhaustive_<T>(name + "_50", 50, generator, containers_all);
#   if PPE_RUN_EXHAUSTIVE_BENCHMARKS || PPE_RUN_BENCHMARK_ONE_CONTAINER
    Benchmark_Containers_Exhaustive_<T>(name + "_60", 60, generator, containers_all);
    Benchmark_Containers_Exhaustive_<T>(name + "_70", 70, generator, containers_all);
    Benchmark_Containers_Exhaustive_<T>(name + "_80", 80, generator, containers_all);
    Benchmark_Containers_Exhaustive_<T>(name + "_90", 90, generator, containers_all);
#   endif
#endif

#if !USE_PPE_ASSERT
    Benchmark_Containers_Exhaustive_<T>(name + "_100", 100, generator, containers_large);
#   if PPE_RUN_EXHAUSTIVE_BENCHMARKS || PPE_RUN_BENCHMARK_ONE_CONTAINER
    Benchmark_Containers_Exhaustive_<T>(name + "_120", 120, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_140", 140, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_160", 160, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_180", 180, generator, containers_large);
#   endif
    Benchmark_Containers_Exhaustive_<T>(name + "_200", 200, generator, containers_large);
#   if PPE_RUN_EXHAUSTIVE_BENCHMARKS || PPE_RUN_BENCHMARK_ONE_CONTAINER
    Benchmark_Containers_Exhaustive_<T>(name + "_220", 220, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_240", 240, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_260", 260, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_280", 280, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_300", 300, generator, containers_large);
#   endif
#endif

    Benchmark_Containers_FindSpeed_<T>(name + "_find", generator, containers_all);

#if !USE_PPE_ASSERT
#   if PPE_RUN_EXHAUSTIVE_BENCHMARKS
    Benchmark_Containers_Exhaustive_<T>(name + "_400", 400, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_500", 500, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_600", 600, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_700", 700, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_800", 800, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_900", 900, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_1000", 1000, generator, containers_large);
#       if USE_PPE_CONTAINERS_LONGRUN
    Benchmark_Containers_Exhaustive_<T>(name + "_5000", 5000, generator, containers_large);
#       endif
#   endif
#endif
}
//----------------------------------------------------------------------------
#if USE_PPE_CONTAINERS_STRING
NO_INLINE static void Test_StringSet_() {
     TRawStorage<char> stringPool;
     stringPool.Resize_DiscardData(64 * 1024); // 64k of text
     FRandomGenerator rnd(42);
     Collect(stringPool.MakeView(), [&](size_t, char* pch) {
         constexpr char Charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-*.$^@#~";
         *pch = Charset[rnd.Next(lengthof(Charset) - 1/* null char */)];
     });

     auto samples = [&](auto& rnd) {
         constexpr u32 MinSize = 5;
         constexpr u32 MaxSize = 60;

 #if 0
         const size_t n = (rnd() % (MaxSize - MinSize + 1)) + MinSize;
         const size_t o = (rnd() % (stringPool.size() - n));
 #else
         const size_t n = rnd(MinSize,  MaxSize);
         const size_t o = rnd(checked_cast<u32>(stringPool.size() - n));
 #endif

         return FStringView(&stringPool[o], n);
     };

     using namespace BenchmarkContainers;

     auto generator = TSamplePool<FStringView>::MakeUniqueGenerator(samples,
         STRINGVIEW_HASHSET(Benchmark, ECase::Sensitive){} );

     auto containers = [](auto& bm, const auto* input) {
         /*{
             typedef TDenseHashSet<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   hashtable_type;

             hashtable_type set;
             bm.Run("Dense", set, input);
         }*//*
         {
             typedef TDenseHashSet<
                 THashMemoizer<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
                 >
             >   hashtable_type;

             hashtable_type set;
             bm.Run("Dense_M", set, input);
         }*/
 #if !PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             typedef TDenseHashSet2<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   hashtable_type;

             hashtable_type set;
             bm.Run("Dense2", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             typedef TDenseHashSet2<
                 THashMemoizer<
                     FStringView,
                     TStringViewHasher<char, ECase::Sensitive>,
                     TStringViewEqualTo<char, ECase::Sensitive>
                 >
             >   hashtable_type;

             hashtable_type set;
             bm.Run("Dense2_M", set, input);
         }
 #   endif
 #endif
 #if 0 && !PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             typedef TDenseHashSet3<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   hashtable_type;

             hashtable_type set;
             bm.Run("DenseHashSet3", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             typedef TDenseHashSet3<
                 THashMemoizer<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
                 >
             >   hashtable_type;

             hashtable_type set;
             bm.Run("DenseHashSet3_M", set, input);
         }
 #   endif
 #endif
 #if !PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             typedef THopscotchHashSet<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   hashtable_type;

             hashtable_type set;
             bm.Run("Hopscotch", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             typedef THopscotchHashSet <
                 THashMemoizer<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
                 >
             >   hashtable_type;

             hashtable_type set;
             bm.Run("Hopscotch_M", set, input);
         }
 #   endif
 #endif
 #if !PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             THopscotchHashSet2<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   set;
             bm.Run("Hopscotch2", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             THopscotchHashSet2<
                 THashMemoizer<
                     FStringView,
                     TStringViewHasher<char, ECase::Sensitive>,
                     TStringViewEqualTo<char, ECase::Sensitive>
                 >
             >   set;
             bm.Run("Hopscotch2_M", set, input);
         }
         /* much slower
         {
             struct FGoodOAAT {
                 hash_t operator ()(const FStringView& s) const NOEXCEPT {
                     return FPlatformHash::GoodOAAT(s.data(), s.SizeInBytes(), PPE_HASH_VALUE_SEED_32);
                 }
             };

             THopscotchHashSet2<
                 FStringView,
                 FGoodOAAT,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   set;
             bm.Run("Hopscotch2_OAAT", set, input);
         }
         */
 #   endif
 #endif
 #if !PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             STRINGVIEW_HASHSET(Container, ECase::Sensitive) set;

             bm.Run("HashSet", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             STRINGVIEW_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

             bm.Run("HashSet_M", set, input);
         }
 #   endif
 #endif
 #if !PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             TSSEHashSet<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   set;

             bm.Run("SSEHashSet", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             TSSEHashSet<
                 THashMemoizer<
                     FStringView,
                     TStringViewHasher<char, ECase::Sensitive>,
                     TStringViewEqualTo<char, ECase::Sensitive> >
             >   set;

             bm.Run("SSEHashSet_M", set, input);
         }
 #   endif
 #endif
 #if 1//!PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             TSSEHashSet2<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   set;

             bm.Run("SSEHashSet2", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             TSSEHashSet2<
                 THashMemoizer<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive> >
             >   set;

             bm.Run("SSEHashSet2_M", set, input);
         }
 #   endif
 #endif
 #if 0//USE_PPE_AVX2//!PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             TSSEHashSet3<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   set;

             bm.Run("SSEHashSet3", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             TSSEHashSet3<
                 THashMemoizer<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive> >
             >   set;

             bm.Run("SSEHashSet3_M", set, input);
         }
 #   endif
 #endif
 #if !PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             TSSEHashSet4<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   set;

             bm.Run("SSEHashSet4", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             TSSEHashSet4<
                 THashMemoizer<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive> >
             >   set;

             bm.Run("SSEHashSet4_M", set, input);
         }
 #   endif
#endif
#if 1//!PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             TSSEHashSet5<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>
             >   set;

             bm.Run("SSEHashSet5", set, input);
         }
#   if USE_PPE_CONTAINERS_MEMOIZER
         {
             TSSEHashSet5<
                 THashMemoizer<
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive> >
             >   set;

             bm.Run("SSEHashSet5_M", set, input);
         }
#   endif
#endif
 #if 0 //!PPE_RUN_BENCHMARK_ONE_CONTAINER
         {
             CONSTCHAR_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

             bm.Run("ConstCharHashSet_M", set, input);
         }
 #endif
 #if !PPE_RUN_BENCHMARK_ONE_CONTAINER && !PPE_DONT_USE_STD_UNORDEREDSET
         {
             std::unordered_set <
                 FStringView,
                 TStringViewHasher<char, ECase::Sensitive>,
                 TStringViewEqualTo<char, ECase::Sensitive>,
                 TStlAllocator < FStringView, ALLOCATOR(Container) >
             >   set;

             bm.Run("unordered_set", set, input);
         }
 #   if USE_PPE_CONTAINERS_MEMOIZER
         {
             std::unordered_set<
                 TBasicStringViewHashMemoizer<char, ECase::Sensitive>,
                 Meta::THash< TBasicStringViewHashMemoizer<char, ECase::Sensitive> >
             >   set;

             bm.Run("unordered_set_M", set, input);
         }
 #   endif
 #endif
     };

     Benchmark_Containers_Exhaustive_<FStringView>("Strings_20", 20, generator, containers);
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS || PPE_RUN_BENCHMARK_ONE_CONTAINER
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_30", 30, generator, containers);
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_40", 40, generator, containers);
#endif
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_50", 50, generator, containers);
 #if PPE_RUN_EXHAUSTIVE_BENCHMARKS || PPE_RUN_BENCHMARK_ONE_CONTAINER
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_60", 60, generator, containers);
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_70", 70, generator, containers);
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_80", 80, generator, containers);
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_90", 90, generator, containers);
 #endif
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_100", 100, generator, containers);
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS || PPE_RUN_BENCHMARK_ONE_CONTAINER
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_150", 150, generator, containers);
#endif
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_200", 200, generator, containers);
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS || PPE_RUN_BENCHMARK_ONE_CONTAINER
     Benchmark_Containers_Exhaustive_<FStringView>("Strings_300", 300, generator, containers);
#endif

     Benchmark_Containers_FindSpeed_<FStringView>("Strings_find", generator, containers);
}
#endif //!USE_PPE_CONTAINERS_STRING
#endif //!USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
NO_INLINE static void Test_StealFromDifferentAllocator_() {
    // steal allocations from different tracking domains
    {
        STATIC_ASSERT(has_stealallocatorblock_v<
            VECTOR(Container, int)::allocator_type,
            VECTOR(Benchmark, int)::allocator_type >);

        VECTOR(Benchmark, int) u = { 1, 2, 3 };
        VECTOR(Container, int) v = u;
        VECTOR(Container, int) w = std::move(u);
    }
    {
        STATIC_ASSERT(has_stealallocatorblock_v<
            HASHSET(Container, int)::allocator_type,
            HASHSET(Benchmark, int)::allocator_type >);

        HASHSET(Benchmark, int) u = { 1, 2, 3 };
        HASHSET(Container, int) w = std::move(u);
    }
}
//----------------------------------------------------------------------------
NO_INLINE void Test_MinMaxHeap_() {
    STACKLOCAL_POD_HEAP(int, Meta::TLess<int>{}, heap, 4);

    int seq[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    FRandomGenerator rng;
    rng.Shuffle(MakeView(seq));

    for (int i : seq)
        heap.Roll(i);

    int vmin;
    VerifyRelease(heap.PopMin(&vmin));

    int vmax;
    VerifyRelease(heap.PopMax(&vmax));

    AssertRelease(vmin == 0);
    AssertRelease(vmax == 3);

    LOG(Test_Containers, Emphasis, L"MinMax heap: {0} -> [{1}, {2}]", heap.MakeView(), vmin, vmax);
}
//----------------------------------------------------------------------------
NO_INLINE void Test_SSEHashSet() {
    TSSEHashSet<int> set;
    set.insert(32);
    TSSEHashSet<int> set2;
    set2 = set;
}
//----------------------------------------------------------------------------
NO_INLINE void Test_Appendable() {
    TVector<int> vector;
    MakeAppendable(vector).push_back(42);
    AssertRelease(vector.back() == 42);
    TSparseArray<int> sparse;
    MakeAppendable(sparse).emplace_back(69);
    AssertRelease(sparse.size() == 1);
    TFixedSizeStack<int, 8> fixed;
    MakeAppendable(fixed) << 1 << 2 << 3;
    AssertRelease(fixed.size() == 3);
}
//----------------------------------------------------------------------------
template <size_t _Dim, typename T = size_t>
static void Test_BitSetImpl_() {
    u32 trueBits[16];
    STATIC_ASSERT(lengthof(trueBits) <= _Dim);
    FRandomGenerator rng;
    forrange(loop, 0, 10) {
        TFixedSizeBitMask<_Dim, T> bits0{ Meta::ForceInit };
        AssertRelease(bits0.AllFalse());
        for (u32& bit : trueBits) {
            for (;;) {
                bit = rng.NextU32(_Dim);
                if (not bits0[bit])
                    break;
            }
            bits0.SetTrue(bit);
        }
        AssertRelease(bits0.Count() == lengthof(trueBits));
        std::sort(std::begin(trueBits), std::end(trueBits));
        TFixedSizeBitMask<_Dim, T> bits{ std::begin(trueBits), std::end(trueBits) };
        AssertRelease(bits.AnyTrue());
        AssertRelease(bits.AnyFalse());
        AssertRelease(bits == bits0);
        TFixedSizeBitMask<_Dim, T> bits2 = bits.Invert();
        for (auto bit : trueBits) {
            AssertRelease(bits[bit]);
            AssertRelease(not bits2[bit]);
            bits.SetFalse(bit);
            bits2.SetTrue(bit);
        }
        AssertRelease(bits.AllFalse());
        AssertRelease(not bits.AnyTrue());
        AssertRelease(bits.AnyFalse());
        bits.SetTrue(_Dim - 1);
        AssertRelease(bits.AnyTrue());
        AssertRelease(not bits.AllFalse());
        AssertRelease(bits2.AllTrue());
        AssertRelease(not bits2.AnyFalse());
        AssertRelease(bits2.AnyTrue());
        bits2.SetFalse(_Dim - 1);
        AssertRelease(not bits2.AllTrue());
        AssertRelease(bits2.AnyFalse());
        TFixedSizeBitMask<_Dim, T> bits3{ std::begin(trueBits), std::end(trueBits) };
        T prev = 0;
        forrange(i, 0, lengthof(trueBits)) {
            auto in = bits3.PopFront(prev);
            AssertRelease(in);
            AssertRelease(bits3.Count() == lengthof(trueBits) - i - 1);
            AssertRelease(trueBits[i] + 1 == in);
            prev = in - 1;
        }
        AssertRelease(bits3.AllFalse());
    }
}
//----------------------------------------------------------------------------
NO_INLINE void Test_BitSet() {
    STATIC_ASSERT(TFixedSizeBitMask<4>{ 0, 1, 2, 3 }.AllTrue());
    Test_BitSetImpl_<31, u32>();
    Test_BitSetImpl_<32, u32>();
    Test_BitSetImpl_<67>();
    Test_BitSetImpl_<64>();
    Test_BitSetImpl_<113>();
    Test_BitSetImpl_<471, u32>();
}
//----------------------------------------------------------------------------
NO_INLINE void Test_TupleVector() {
    struct dummy_t_ {
        int data;
        bool operator ==(const dummy_t_& other) const NOEXCEPT { return data == other.data; }
    };

    TUPLEVECTOR_INSITU(Container, 8, int, dummy_t_, double) vec;

    vec.resize(5, decltype(vec)::value_type{});
    vec.emplace_back(1, dummy_t_{ 6 }, 2.1);
    vec.emplace_back(2, dummy_t_{ 9 }, 3.2);

    decltype(vec)::value_type value{ 3, dummy_t_{12}, 4.3 };
    vec.insert(vec.begin() + 2, std::move(value));

    const int intSum = std::accumulate(vec.MakeView<0>().begin(), vec.MakeView<0>().end(), 0);
    VerifyRelease(6 == intSum);

    const double dblSum = std::accumulate(vec.MakeView<2>().begin(), vec.MakeView<2>().end(), 0.0);
    VerifyRelease(NearlyEquals(9.6, dblSum));

    const decltype(vec) vec2{ vec };
    forrange(i, 0, vec.size())
        VerifyRelease(vec2.at(i) == vec.at(i));

    int dummySum = 0;
    for (const auto& jt : vec2)
        dummySum += std::get<1>(jt).data;
    VerifyRelease(27 == dummySum);

    int intSum2 = 0;
    int dummySum2 = 0;
    for (const auto& jt : vec2.MakeIterable<1, 0>()) {
        dummySum2 += std::get<0>(jt).data;
        intSum2 += std::get<1>(jt);
    }
    VerifyRelease(intSum2 == intSum);
    VerifyRelease(dummySum2 == dummySum);

    TUPLEVECTOR(Container, int, dummy_t_, double) vec3{ vec };
    AssertRelease(vec2.size() == vec3.size());
    VerifyRelease(std::equal(vec.begin(), vec.end(), vec3.begin(), vec3.end() ));
    VerifyRelease(vec.MakeIterable<2, 0, 1>().Equals(vec3.MakeIterable<2, 0, 1>()));

    /*
    LINEARHEAP_POOLED(Container) heap;
    TUPLEVECTOR_LINEARHEAP(int, dummy_t_, double) vec4{ vec3, FLinearAllocator(heap) };
    VerifyRelease(vec4.MakeIterable<2, 0, 1>().Equals(vec4.MakeIterable<2, 0, 1>()));
    */
}
//----------------------------------------------------------------------------
void Test_Containers() {
    PPE_DEBUG_NAMEDSCOPE("Test_Containers");

    LOG(Test_Containers, Emphasis, L"starting container tests ...");

    EProcessPriority prio = EProcessPriority::Normal;
    Verify(FPlatformProcess::Priority(&prio, FPlatformProcess::CurrentProcess()));
    Verify(FPlatformProcess::SetPriority(FPlatformProcess::CurrentProcess(), EProcessPriority::Realtime));

    ON_SCOPE_EXIT([prio]() {
        FPlatformProcess::SetPriority(FPlatformProcess::CurrentProcess(), prio);
    });

    Test_StealFromDifferentAllocator_();
    Test_MinMaxHeap_();
    Test_SSEHashSet();
    Test_Appendable();
    Test_BitSet();
    Test_TupleVector();

#if USE_PPE_BENCHMARK
    Test_PODSet_<u64>("u64", [](auto& rnd) { return u64(rnd()); });
#   if PPE_RUN_EXHAUSTIVE_BENCHMARKS
    Test_PODSet_<u32>("u32", [](auto& rnd) { return u32(rnd()); });
    Test_PODSet_<u128>("u128", [](auto& rnd) { return u128{ u64(rnd()), u64(rnd()) }; });
    Test_PODSet_<u256>("u256", [](auto& rnd) { return u256{ { u64(rnd()), u64(rnd()) }, { u64(rnd()), u64(rnd()) } }; });
#   endif
#   if USE_PPE_CONTAINERS_STRING
    Test_StringSet_();
#   endif
#endif

    FLUSH_LOG();
    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
