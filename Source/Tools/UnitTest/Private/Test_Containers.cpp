#include "stdafx.h"

#include "Container/AssociativeVector.h"
#include "Container/BurstTrie.h"
#include "Container/FixedSizeHashSet.h"
#include "Container/FlatMap.h"
#include "Container/FlatSet.h"
#include "Container/HashTable.h"
#include "Container/StringHashSet.h"

#include "Diagnostic/Benchmark.h"
#include "Diagnostic/Logger.h"

#include "Allocator/LinearHeap.h"
#include "IO/BufferedStream.h"
#include "IO/FileStream.h"
#include "IO/Filename.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "VirtualFileSystem.h"
#include "HAL/PlatformMaths.h"
#include "Maths/Maths.h"
#include "Maths/PrimeNumbers.h"
#include "Maths/RandomGenerator.h"
#include "Memory/MemoryStream.h"
#include "Meta/PointerWFlags.h"
#include "Time/TimedScope.h"

#include <random>
#include <unordered_set>

namespace PPE {
LOG_CATEGORY(, Test_Containers)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template class TAssociativeVector<FString, int>;
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

namespace PPE {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
namespace BenchmarkContainers {
template <typename T>
struct TInputData {
    TMemoryView<const T> Insert;
    TMemoryView<const T> Unkown;
    TMemoryView<const T> Search;
    TMemoryView<const T> Erase;
    TMemoryView<const T> Dense;
    TMemoryView<const T> Sparse;
    TMemoryView<const u32> Shuffled;

    template <typename _Container>
    void FillDense(_Container& c) const {
        c.reserve(Insert.size());
#ifdef WITH_PPE_ASSERT
        forrange(i, 0, Insert.size()) {
            c.insert(Insert[i]);
            forrange(j, 0, i+1)
                Assert_NoAssume(c.find(Insert[j]) != c.end());
        }

#else
        for (const auto& it : Insert)
            c.insert(it);
#endif
    }

    template <typename _Container>
    void FillSparse(_Container& c) const {
        FillDense(c);
        for (const auto& it : Sparse)
            Verify(c.erase(it));
    }

};
class construct_noreserve_t : public FBenchmark {
public:
    construct_noreserve_t() : FBenchmark{ "ctor_cold" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        for (auto _ : state) {
            auto c{ archetype };
            for (const auto& it : input.Insert)
                c.insert(it);
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class construct_reserve_t : public FBenchmark {
public:
    construct_reserve_t() : FBenchmark{ "ctor_warm" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        for (auto _ : state) {
            auto c{ archetype };
            c.reserve(input.Insert.size());
            for (const auto& it : input.Insert)
                c.insert(it);
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class copy_empty_t : public FBenchmark {
public:
    copy_empty_t() : FBenchmark{ "copy_pty" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };

        for (auto _ : state) {
            c.clear();
            state.ResetTiming();
            c = s;
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class copy_dense_t : public FBenchmark {
public:
    copy_dense_t() : FBenchmark{ "copy_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ s };

        for (auto _ : state) {
            c = s;
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class copy_sparse_t : public FBenchmark {
public:
    copy_sparse_t() : FBenchmark{ "copy_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        auto c{ s };

        for (auto _ : state) {
            c = s;
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class insert_dense_t : public FBenchmark {
public:
    insert_dense_t() : FBenchmark{ "insert_dns" } {
        //BatchSize = 16; // batching for better accuracy
    }
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        for (auto _ : state) {
            state.PauseTiming();
            auto c = s;
            const auto& item = state.Random().RandomElement(input.Unkown);
            state.ResumeTiming();
            c.insert(item);
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class insert_sparse_t : public FBenchmark {
public:
    insert_sparse_t() : FBenchmark{ "insert_spr" } {
        //BatchSize = 16; // batching for better accuracy
    }
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        for (auto _ : state) {
            state.PauseTiming();
            auto c = s;
            const auto& item = state.Random().RandomElement(input.Unkown);
            state.ResumeTiming();
            c.insert(item);
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class iterate_dense_t : public FBenchmark {
public:
    iterate_dense_t() : FBenchmark{ "iter_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillDense(c);

        for (auto _ : state) {
            for (const auto& it : c)
                FBenchmark::DoNotOptimizeLoop(it);
            FBenchmark::ClobberMemory();
        }
    }
};
class iterate_sparse_t : public FBenchmark {
public:
    iterate_sparse_t() : FBenchmark{ "iter_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        for (auto _ : state) {
            for (const auto& it : c)
                FBenchmark::DoNotOptimizeLoop(it);
            FBenchmark::ClobberMemory();
        }
    }
};
class find_dense_pos_t : public FBenchmark {
public:
    find_dense_pos_t() : FBenchmark{ "find_dns_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillDense(c);

        for (auto _ : state) {
            for (const auto& it : input.Search)
                FBenchmark::DoNotOptimizeLoop(c.find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_dense_neg_t : public FBenchmark {
public:
    find_dense_neg_t() : FBenchmark{ "find_dns_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        for (auto _ : state) {
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_sparse_pos_t : public FBenchmark {
public:
    find_sparse_pos_t() : FBenchmark{ "find_spr_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        for (auto _ : state) {
            for (const auto& it : input.Dense)
                FBenchmark::DoNotOptimizeLoop(c.find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_sparse_neg_t : public FBenchmark {
public:
    find_sparse_neg_t() : FBenchmark{ "find_spr_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        for (auto _ : state) {
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
// trying to measure cache misses by avoiding temporal coherency
// https://www.youtube.com/watch?v=M2fKMP47slQ
class find_cmiss_pos_t : public FBenchmark {
public:
    find_cmiss_pos_t() : FBenchmark{ "find_miss_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillDense(c);

        STACKLOCAL_STACK(_Container, tables, 128);
        forrange(i, 0, 128)
            tables.Push(c);

        AssertRelease(input.Unkown.size() == input.Shuffled.size());

        for (auto _ : state) {
            const u32* pIndex = input.Shuffled.data();
            for (const auto& it : input.Dense)
                FBenchmark::DoNotOptimizeLoop(tables[*(pIndex++) & 127].find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_cmiss_neg_t : public FBenchmark {
public:
    find_cmiss_neg_t() : FBenchmark{ "find_miss_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto c{ archetype };
        input.FillSparse(c);

        STACKLOCAL_STACK(_Container, tables, 128);
        forrange(i, 0, 128)
            tables.Push(c);

        AssertRelease(input.Unkown.size() == input.Shuffled.size());

        for (auto _ : state) {
            const u32* pIndex = input.Shuffled.data();
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(tables[*(pIndex++) & 127].find(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_dense_pos_t : public FBenchmark {
public:
    erase_dense_pos_t() : FBenchmark{ "erase_dns_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            for (const auto& it : input.Search)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_dense_neg_t : public FBenchmark {
public:
    erase_dense_neg_t() : FBenchmark{ "erase_dns_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_sparse_pos_t : public FBenchmark {
public:
    erase_sparse_pos_t() : FBenchmark{ "erase_spr_+" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            for (const auto& it : input.Dense)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class erase_sparse_neg_t : public FBenchmark {
public:
    erase_sparse_neg_t() : FBenchmark{ "erase_spr_-" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            for (const auto& it : input.Unkown)
                FBenchmark::DoNotOptimizeLoop(c.erase(it));
            FBenchmark::ClobberMemory();
        }
    }
};
class find_erase_dns_t : public FBenchmark {
public:
    find_erase_dns_t() : FBenchmark{ "find_erase_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        const size_t ns = input.Sparse.size();
        const size_t nu = input.Unkown.size();
        const size_t nn = Min(nu, ns) >> 1;

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            forrange(i, 0, nn) {
                c.insert(input.Unkown[(i << 1)]);
                c.insert(input.Unkown[(i << 1) + 1]);
                c.erase(input.Sparse[i]);
            }
            FBenchmark::DoNotOptimize(c.end());
            FBenchmark::ClobberMemory();
        }
    }
};
class find_erase_spr_t : public FBenchmark {
public:
    find_erase_spr_t() : FBenchmark{ "find_erase_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        const size_t ns = input.Dense.size();
        const size_t nu = input.Unkown.size();
        const size_t nn = Min(nu, ns) >> 1;

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            forrange(i, 0, nn) {
                c.insert(input.Unkown[(i << 1)]);
                c.insert(input.Unkown[(i << 1) + 1]);
                c.erase(input.Dense[i]);
            }
            FBenchmark::DoNotOptimize(c.end());
            FBenchmark::ClobberMemory();
        }
    }
};
class clear_dense_t : public FBenchmark {
public:
    clear_dense_t() : FBenchmark{ "clear_dns" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillDense(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            c.clear();
            FBenchmark::DoNotOptimize(c);
        }
    }
};
class clear_sparse_t : public FBenchmark {
public:
    clear_sparse_t() : FBenchmark{ "clear_spr" } {}
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TInputData<T>& input) const {
        auto s{ archetype };
        input.FillSparse(s);

        auto c{ archetype };
        for (auto _ : state) {
            c = s;
            state.ResetTiming();
            c.clear();
            FBenchmark::DoNotOptimize(c);
        }
    }
};
} //!namespace BenchmarkContainers
//----------------------------------------------------------------------------
#define PPE_RUN_EXHAUSTIVE_BENCHMARKS (1)
template <typename T, typename _Generator, typename _Containers>
static void Benchmark_Containers_Exhaustive_(
    const FStringView& name, size_t dim,
    _Generator&& generator,
    _Containers&& tests ) {
#if !PPE_RUN_EXHAUSTIVE_BENCHMARKS
    UNUSED(name);
    UNUSED(dim);
    UNUSED(generator);
    UNUSED(tests);
#else
    // prepare input data

    LOG(Benchmark, Emphasis, L"Running benchmark <{0}> with {1} tests :", name, dim);

    // mt19937 has better distribution than FRandomGenerator for generating benchmark data
    std::random_device rdevice;
    std::mt19937 rand{ rdevice() };
    rand.seed(0x32F9u); // fixed seed for repro

    VECTOR(Benchmark, T) samples;
    samples.reserve_Additional(dim * 2);
    forrange(i, 0, dim * 2)
        samples.emplace_back(generator(rand));

    std::shuffle(samples.begin(), samples.end(), rand);

    const TMemoryView<const T> insert = samples.MakeConstView().CutBefore(dim);
    const TMemoryView<const T> unkown = samples.MakeConstView().CutStartingAt(dim);

    constexpr size_t sparse_factor = 70;
    VECTOR(Benchmark, T) search { insert };
    VECTOR(Benchmark, T) erase { search };
    VECTOR(Benchmark, T) sparse {
        erase
            .MakeConstView()
            .CutBeforeConst((sparse_factor * erase.size()) / 100)
    };
    VECTOR(Benchmark, T) dense {
        erase
            .MakeConstView()
            .CutStartingAt(sparse.size())
    };

    std::shuffle(std::begin(erase), std::end(erase), rand);
    std::shuffle(std::begin(search), std::end(search), rand);
    std::shuffle(std::begin(dense), std::end(dense), rand);
    std::shuffle(std::begin(sparse), std::end(sparse), rand);

    VECTOR(Benchmark, u32) shuffled;
    shuffled.resize_Uninitialized(unkown.size());
    forrange(i, 0, checked_cast<u32>(unkown.size()))
        shuffled[i] = i;
    std::shuffle(std::begin(shuffled), std::end(shuffled), rand);

    using namespace BenchmarkContainers;

    using FInputData = TInputData<T>;
    FInputData input{ insert, search, unkown, erase, dense, sparse, shuffled };
    Assert_NoAssume(insert.size());
    Assert_NoAssume(search.size());
    Assert_NoAssume(unkown.size());
    Assert_NoAssume(erase.size());
    Assert_NoAssume(dense.size());
    Assert_NoAssume(sparse.size());
    Assert_NoAssume(shuffled.size() == unkown.size());

    // prepare benchmark table

    auto bm = FBenchmark::MakeTable(
        name,
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
        find_cmiss_pos_t{},
        find_cmiss_neg_t{},
        find_erase_dns_t{},
        find_erase_spr_t{},
        erase_dense_pos_t{},
        erase_dense_neg_t{},
        erase_sparse_pos_t{},
        erase_sparse_neg_t{},
        clear_dense_t{},
        clear_sparse_t{} );

    tests(bm, input);

    FBenchmark::Log(bm);

    {
        const FFilename fname{ StringFormat(L"Saved:/Benchmark/{0}/Containers/{1}.csv",
            MakeStringView(WSTRINGIZE(BUILDCONFIG)), name) };

        FStringBuilder sb;
        FBenchmark::Csv(bm, sb);
        FString s{ sb.ToString() };
        VFS_WriteAll(fname, s.MakeView().RawView(), EAccessPolicy::Truncate_Binary|EAccessPolicy::Roll);
    }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
}
//----------------------------------------------------------------------------
namespace BenchmarkContainers {
template <typename T>
struct TFindData {
    STATIC_CONST_INTEGRAL(u32, Entropy, 128);
    struct FSeries {
        TMemoryView<const T> Insert;
        TMemoryView<const T> Unknown;
    };
    FSeries Series[Entropy];
    TMemoryView<const u32> Shuffled;
};
class findspeed_dense_pos_t : public FBenchmark {
public:
    findspeed_dense_pos_t(size_t n, const FStringView& name) : FBenchmark{ name } {
        InputDim = checked_cast<u32>(n);
    }
    template <typename _Container, typename T>
    void operator ()(FBenchmark::FState& state, const _Container& archetype, const TFindData<T>& input) const {
        Assert_NoAssume(input.Shuffled.size() >= InputDim);

        STACKLOCAL_STACK(_Container, tables, TFindData<T>::Entropy);
        forrange(i, 0, TFindData<T>::Entropy) {
            Assert_NoAssume(input.Series[i].Insert.size() >= InputDim);

            tables.Push(archetype);
            auto& h = *tables.Peek();

            h.reserve(InputDim);
            Assert_NoAssume(h.capacity() >= InputDim);
            for (const auto& it : input.Series[i].Insert.CutBefore(InputDim))
                h.insert(it);
        }

        for (auto _ : state) {
            uintptr_t hit = 0;
            const u32* pIndex = input.Shuffled.data();
            forrange(i, 0, InputDim) {
                const u32 hi = *(pIndex++);
                hit ^= uintptr_t(&*(tables[hi].find(input.Series[hi].Insert[i])));
            }
            FBenchmark::DoNotOptimize(hit);
        }
    }
};
} //!namespace BenchmarkContainers
template <typename T, typename _Generator, typename _Containers>
static void Benchmark_Containers_FindSpeed_(const FStringView& name, _Generator&& generator, _Containers&& tests) {
    constexpr size_t dim = 4200;

    LOG(Benchmark, Emphasis, L"Running find benchmarks <{0}> with {1} tests :", name, dim);

    // mt19937 has better distribution than FRandomGenerator for generating benchmark data
    std::random_device rdevice;
    std::mt19937 rand{ rdevice() };
    rand.seed(0x2875u); // fixed seed for repro

    using namespace BenchmarkContainers;

    CONSTEXPR const size_t samplesPerSeries = dim * 2;
    CONSTEXPR const size_t totalSamples = samplesPerSeries * TFindData<T>::Entropy;

    VECTOR(Benchmark, T) samples;
    samples.reserve_AssumeEmpty(totalSamples);
    forrange(i, 0, totalSamples)
        samples.emplace_back(generator(rand));

    std::shuffle(samples.begin(), samples.end(), rand);

    VECTOR(Benchmark, u32) shuffled;
    shuffled.resize_Uninitialized(dim);
    forrange(i, 0, dim)
        shuffled[i] = i % TFindData<T>::Entropy;
    std::shuffle(shuffled.begin(), shuffled.end(), rand);

    TFindData<T> input;
    input.Shuffled = shuffled;

    forrange(i, 0, TFindData<T>::Entropy) {
        const size_t off = dim * 2 * i;
        input.Series[i] = {
            samples.MakeConstView().SubRange(off, dim),
            samples.MakeConstView().SubRange(off + dim, dim) };
    }

    auto bm = FBenchmark::MakeTable(
        name,
        findspeed_dense_pos_t{ 4, "4" },
        findspeed_dense_pos_t{ 6, "6" },
        findspeed_dense_pos_t{ 9, "9" },
        findspeed_dense_pos_t{ 12, "12" },
        findspeed_dense_pos_t{ 16, "16" },
        findspeed_dense_pos_t{ 20, "20" },
        findspeed_dense_pos_t{ 25, "25" },
        findspeed_dense_pos_t{ 30, "30" },
        findspeed_dense_pos_t{ 36, "36" },
        findspeed_dense_pos_t{ 42, "42" },
        findspeed_dense_pos_t{ 49, "49" },
        findspeed_dense_pos_t{ 56, "56" },
        findspeed_dense_pos_t{ 64, "64" },
        findspeed_dense_pos_t{ 72, "72" },
        findspeed_dense_pos_t{ 81, "81" },
        findspeed_dense_pos_t{ 90, "90" },
        findspeed_dense_pos_t{ 100, "100" },
        findspeed_dense_pos_t{ 110, "110" },
        findspeed_dense_pos_t{ 121, "121" },
        findspeed_dense_pos_t{ 132, "132" },
        findspeed_dense_pos_t{ 144, "144" },
        findspeed_dense_pos_t{ 156, "156" },
        findspeed_dense_pos_t{ 169, "169" },
        findspeed_dense_pos_t{ 182, "182" },
        findspeed_dense_pos_t{ 196, "196" }
#ifndef WITH_PPE_ASSERT
        ,
        findspeed_dense_pos_t{ 210, "210" },
        findspeed_dense_pos_t{ 225, "225" },
        findspeed_dense_pos_t{ 240, "240" },
        findspeed_dense_pos_t{ 256, "256" },
        findspeed_dense_pos_t{ 272, "272" },
        findspeed_dense_pos_t{ 289, "289" },
        findspeed_dense_pos_t{ 306, "306" },
        findspeed_dense_pos_t{ 324, "324" },
        findspeed_dense_pos_t{ 342, "342" },
        findspeed_dense_pos_t{ 361, "361" },
        findspeed_dense_pos_t{ 380, "380" },
        findspeed_dense_pos_t{ 400, "400" },
        findspeed_dense_pos_t{ 420, "420" },
        findspeed_dense_pos_t{ 441, "441" },
        findspeed_dense_pos_t{ 462, "462" },
        findspeed_dense_pos_t{ 484, "484" }
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS
        ,
        findspeed_dense_pos_t{ 506, "506" },
        findspeed_dense_pos_t{ 529, "529" },
        findspeed_dense_pos_t{ 552, "552" },
        findspeed_dense_pos_t{ 576, "576" },
        findspeed_dense_pos_t{ 600, "600" },
        findspeed_dense_pos_t{ 625, "625" },
        findspeed_dense_pos_t{ 650, "650" },
        findspeed_dense_pos_t{ 676, "676" },
        findspeed_dense_pos_t{ 702, "702" },
        findspeed_dense_pos_t{ 729, "729" },
        findspeed_dense_pos_t{ 756, "756" },
        findspeed_dense_pos_t{ 784, "784" },
        findspeed_dense_pos_t{ 812, "812" },
        findspeed_dense_pos_t{ 841, "841" },
        findspeed_dense_pos_t{ 870, "870" },
        findspeed_dense_pos_t{ 900, "900" },
        findspeed_dense_pos_t{ 930, "930" },
        findspeed_dense_pos_t{ 961, "961" },
        findspeed_dense_pos_t{ 992, "992" },
        findspeed_dense_pos_t{ 1024, "1024" },
        findspeed_dense_pos_t{ 1056, "1056" },
        findspeed_dense_pos_t{ 1089, "1089" },
        findspeed_dense_pos_t{ 1122, "1122" }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
#endif //!WITH_PPE_ASSERT
    );

    tests(bm, input);

    FBenchmark::Log(bm);

    {
        const FFilename fname{ StringFormat(L"Saved:/Benchmark/{0}/Containers/{1}.csv",
            MakeStringView(WSTRINGIZE(BUILDCONFIG)), name) };

        FStringBuilder sb;
        FBenchmark::Csv(bm, sb);
        FString s{ sb.ToString() };
        VFS_WriteAll(fname, s.MakeView().RawView(), EAccessPolicy::Truncate_Binary | EAccessPolicy::Roll);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Generator>
static void Test_PODSet_(const FString& name, const _Generator& generator) {
    auto containers_large = [](auto& bm, const auto& input) {
        /*{ // very buggy
            typedef TCompactHashSet<T> hashtable_type;

            hashtable_type set;
            bm.Run("TCompactHashSet", set, input);
        }*/
        {
            typedef TDenseHashSet2<T> hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet2", set, input);
        }
        {
            typedef TDenseHashSet3<T> hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet3", set, input);
        }
#if 0
        {
            typedef TDenseHashSet2<THashMemoizer<T>> hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet2_M", set, input);
        }
        {
            typedef TDenseHashSet3<THashMemoizer<T>> hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet3_M", set, input);
        }
#endif
        {
            THashSet<T> set;

            bm.Run("THashSet", set, input);
        }
        {
            std::unordered_set<T, Meta::THash<T>, Meta::TEqualTo<T>, ALLOCATOR(Container, T)> set;

            bm.Run("unordered_set", set, input);
        }
    };

    auto containers_all = [&](auto& bm, const auto& input) {
#if PPE_RUN_EXHAUSTIVE_BENCHMARKS
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

            bm.Run("TVector", set, input);
        }
#endif //!PPE_RUN_EXHAUSTIVE_BENCHMARKS
        {
            TFlatSet<T> set;

            bm.Run("TFlatSet", set, input);
        }
        {
            TFixedSizeHashSet<T, 2048> set;

            bm.Run("TFixedSizeHashSet", set, input);
        }
        containers_large(bm, input);
    };

    Benchmark_Containers_FindSpeed_<T>(name + "_find", generator, containers_all);

    Benchmark_Containers_Exhaustive_<T>(name + "_20", 20, generator, containers_all);
    Benchmark_Containers_Exhaustive_<T>(name + "_50", 50, generator, containers_all);
    Benchmark_Containers_Exhaustive_<T>(name + "_200", 200, generator, containers_large);
#ifndef WITH_PPE_ASSERT
    Benchmark_Containers_Exhaustive_<T>(name + "_2000", 2000, generator, containers_large);
    Benchmark_Containers_Exhaustive_<T>(name + "_20000", 20000, generator, containers_large);
#endif
}
//----------------------------------------------------------------------------
static void Test_StringSet_() {
    TRawStorage<char> stringPool;
    stringPool.Resize_DiscardData(64 * 1024); // 64k of text
    FRandomGenerator rnd(42);
    stringPool.MakeView().Collect([&](size_t, char* pch) {
        constexpr char Charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-*.$^@#~";
        *pch = Charset[rnd.Next(lengthof(Charset) - 1/* null char */)];
    });

    auto generator = [&](auto& rnd) {
        constexpr size_t MinSize = 5;
        constexpr size_t MaxSize = 60;

        const size_t n = (rnd() % (MaxSize - MinSize + 1)) + MinSize;
        const size_t o = (rnd() % (stringPool.size() - n));

        return FStringView(&stringPool[o], n);
    };

    auto containers = [](auto& bm, const auto& input) {
        /*{
            typedef TDenseHashSet<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            bm.Run("DenseHashSet", set, input);
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
            bm.Run("DenseHashSet_M", set, input);
        }*/
        {
            typedef TDenseHashSet2<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet2", set, input);
        }
        {
            typedef TDenseHashSet2<
                THashMemoizer<
                    FStringView,
                    TStringViewHasher<char, ECase::Sensitive>,
                    TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet2_M", set, input);
        }
        {
            typedef TDenseHashSet3<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
            >   hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet3", set, input);
        }
        {
            typedef TDenseHashSet3<
                THashMemoizer<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>
                >
            >   hashtable_type;

            hashtable_type set;
            bm.Run("TDenseHashSet3_M", set, input);
        }
        {
            STRINGVIEW_HASHSET(Container, ECase::Sensitive) set;

            bm.Run("THashSet", set, input);
        }
        {
            STRINGVIEW_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

            bm.Run("THashSet_M", set, input);
        }
        /*{
            CONSTCHAR_HASHSET_MEMOIZE(Container, ECase::Sensitive) set;

            bm.Run("TConstCharHashSet_M", set, input);
        }*/
        {
            std::unordered_set<
                FStringView,
                TStringViewHasher<char, ECase::Sensitive>,
                TStringViewEqualTo<char, ECase::Sensitive>,
                ALLOCATOR(Container, FStringView)
            >   set;

            bm.Run("unordered_set", set, input);
        }
        {
            std::unordered_set<
                TBasicStringViewHashMemoizer<char, ECase::Sensitive>,
                Meta::THash< TBasicStringViewHashMemoizer<char, ECase::Sensitive> >
            >   set;

            bm.Run("unordered_set_M", set, input);
        }
    };

    Benchmark_Containers_FindSpeed_<FStringView>("Strings_find", generator, containers);

    Benchmark_Containers_Exhaustive_<FStringView>("Strings_20", 20, generator, containers);
    Benchmark_Containers_Exhaustive_<FStringView>("Strings_50", 50, generator, containers);
#ifndef WITH_PPE_ASSERT
    Benchmark_Containers_Exhaustive_<FStringView>("Strings_200", 200, generator, containers);
    Benchmark_Containers_Exhaustive_<FStringView>("Strings_2000", 2000, generator, containers);
#endif
}
#endif //!USE_PPE_BENCHMARK
//----------------------------------------------------------------------------
static void Test_StealFromDifferentAllocator_() {
    // steal allocations from different tracking domains
    {
        VECTOR(NativeTypes, int) u = { 1, 2, 3 };
        VECTOR(Container, int) v = u;
        VECTOR(Container, int) w = std::move(u);
    }
    {
        HASHSET(NativeTypes, int) u = { 1, 2, 3 };
        HASHSET(Container, int) w = std::move(u);
    }
}
//----------------------------------------------------------------------------
void Test_Containers() {
    LOG(Test_Containers, Emphasis, L"starting container tests ...");

    Test_StealFromDifferentAllocator_();

#if USE_PPE_BENCHMARK
    Test_PODSet_<u32>("u32", [](auto& rnd) { return u32(rnd()); });
    Test_PODSet_<u64>("u64", [](auto& rnd) { return u64(rnd()); });
    Test_PODSet_<u128>("u128", [](auto& rnd) { return u128{ u64(rnd()), u64(rnd()) }; });
    Test_PODSet_<u256>("u256", [](auto& rnd) { return u256{ { u64(rnd()), u64(rnd()) }, { u64(rnd()), u64(rnd()) } }; });
    Test_StringSet_();
#endif

    FLUSH_LOG();
    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
