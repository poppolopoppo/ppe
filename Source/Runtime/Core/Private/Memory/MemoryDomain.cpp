#include "stdafx.h"

#include "Memory/MemoryDomain.h"


#include "Allocator/InitSegAllocator.h"
#include "Memory/MemoryTracking.h"
#include "Memory/MemoryView.h"

#include "Diagnostic/Logger.h"
#include "Meta/OneTimeInitialize.h"

#if !USE_PPE_FINAL_RELEASE
#   include "Allocator/InitSegAllocator.h"
#   include "Container/IntrusiveList.h"
#   include "Container/Stack.h"
#   include "Diagnostic/CurrentProcess.h"
#   include "Diagnostic/Logger.h"
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#   include "IO/String.h"
#   include "IO/StringBuilder.h"
#   include "IO/TextWriter.h"
#   include "Thread/AtomicSpinLock.h"

#   include <algorithm>
#endif

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, MemoryDomain)
#if USE_PPE_MEMORYDOMAINS
CONSTEXPR size_t MemoryDomainsMaxCount = 300;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryDomain : public FMemoryTracking {
public:
    FMemoryDomain(const char* name, FMemoryTracking* parent, EMode mode) NOEXCEPT
    :   FMemoryTracking(name, parent, mode) {
        RegisterTrackingData(this);
    }

    ~FMemoryDomain() NOEXCEPT {
        UnregisterTrackingData(this);
    }

    FMemoryDomain(const FMemoryDomain&) = delete;
    FMemoryDomain& operator =(const FMemoryDomain&) = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MemoryDomain {
    FMemoryTracking& MEMORYDOMAIN_NAME(GpuMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "GpuMemory", nullptr, FMemoryTracking::Recursive);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(PooledMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "PooledMemory", nullptr, FMemoryTracking::Recursive);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(ReservedMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "ReservedMemory", nullptr, FMemoryTracking::Recursive);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(UsedMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "UsedMemory", nullptr, FMemoryTracking::Recursive);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(UnaccountedMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "UnaccountedMemory", nullptr, FMemoryTracking::Recursive);
        return GInstance;
    }
}
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
// Pass for definition of everything
#if !WITH_PPE_MEMORYDOMAINS_FULL_COLLAPSING
#   define MEMORYDOMAIN_GROUP_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        const FMemoryTracking& MEMORYDOMAIN_NAME(_Name)::TrackingData() { \
            auto& parent = const_cast<FMemoryTracking&>(MEMORYDOMAIN_TRACKING_DATA(_Parent)); \
            ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, STRINGIZE(_Name), &parent, FMemoryTracking::Recursive); \
            return GInstance; \
        } \
    }
#   define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        FMemoryTracking& MEMORYDOMAIN_NAME(_Name)::TrackingData() { \
            auto& parent = const_cast<FMemoryTracking&>(MEMORYDOMAIN_TRACKING_DATA(_Parent)); \
            ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, STRINGIZE(_Name), &parent, FMemoryTracking::Recursive); \
            return GInstance; \
        } \
    }
#   define MEMORYDOMAIN_DETAILLED_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        FMemoryTracking& MEMORYDOMAIN_NAME(_Name)::TrackingData() { \
            auto& parent = const_cast<FMemoryTracking&>(MEMORYDOMAIN_TRACKING_DATA(_Parent)); \
            ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, STRINGIZE(_Name), &parent, FMemoryTracking::Isolated); \
            return GInstance; \
        } \
    }
#endif
#if WITH_PPE_MEMORYDOMAINS_COLLAPSING
#   define MEMORYDOMAIN_COLLAPSABLE_IMPL(_Name, _Parent)
#endif
#include "Memory/MemoryDomain.Definitions-inl.h"
#undef MEMORYDOMAIN_COLLAPSABLE_IMPL
#undef MEMORYDOMAIN_GROUP_IMPL
#undef MEMORYDOMAIN_DETAILLED_IMPL
#undef MEMORYDOMAIN_IMPL
//----------------------------------------------------------------------------
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
class FTrackingDataRegistry_ : Meta::FNonCopyableNorMovable {
public:
    static FTrackingDataRegistry_& Get() {
        ONE_TIME_INITIALIZE(TInitSegAlloc<FTrackingDataRegistry_>, GInstance, 10/* very late */);
        return GInstance;
    }

    FTrackingDataRegistry_() = default;

    ~FTrackingDataRegistry_() {
        Assert_NoAssume(_domains.empty());
        _domains.Clear();
    }

    void Register(FMemoryTracking* pTrackingData) {
        Assert(pTrackingData);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        _domains.PushHead(pTrackingData);
    }

    void Unregister(FMemoryTracking* pTrackingData) {
        Assert(pTrackingData);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        _domains.Erase(pTrackingData);
    }

    using FMemoryDomainsList = TFixedSizeStack<const FMemoryTracking*, MemoryDomainsMaxCount>;

    void FetchDatas(const TAppendable<const FMemoryTracking*>& plist) const {
        const FAtomicSpinLock::FScope scopeLock(_barrier);

        for (FMemoryTracking* node = _domains.Head(); node; node = node->Node.Next)
            plist.push_back(node);
    }

private:
    mutable FAtomicSpinLock _barrier;
    INTRUSIVELIST(&FMemoryTracking::Node) _domains;
};
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
void ReportTrackingDatas_(
    FWTextWriter& oss,
    const wchar_t *header,
    const TMemoryView<const FMemoryTracking * const>& datas) {
    Assert(header);

    if (datas.empty())
        return;

    const FWTextWriter::FFormatScope formatScope(oss);

    oss << FTextFormat::Float(FTextFormat::FixedFloat, 2);

    oss << L"  Reporting tracking data :" << Eol;

    CONSTEXPR size_t width = 194;
    CONSTEXPR wchar_t fmtTitle[] = L" {0:/-35}";
    CONSTEXPR wchar_t fmtSnapshot[] = L"| {0:6} {1:8} | {2:9} {3:9} | {4:10} {5:10}   ";
    CONSTEXPR wchar_t fmtFooter[] = L"|| {0:10} {1:10}   ";

    const auto hr = Fmt::Repeat(L'-', width);

    oss << hr << Eol
        << L"    " << header << L" (" << datas.size() << L" elements)" << Eol
        << hr << Eol;

    Format(oss, fmtTitle, L"Tracking domain");
    Format(oss, fmtSnapshot,
        Fmt::AlignCenter(MakeStringView(L"User")),
        Fmt::AlignCenter(MakeStringView(L"Max")),

        Fmt::AlignCenter(MakeStringView(L"Stride")),
        Fmt::AlignCenter(MakeStringView(L"Max")),

        Fmt::AlignCenter(MakeStringView(L"Size")),
        Fmt::AlignCenter(MakeStringView(L"Peak"))
        );
    Format(oss, fmtSnapshot,
        Fmt::AlignCenter(MakeStringView(L"System")),
        Fmt::AlignCenter(MakeStringView(L"Max")),

        Fmt::AlignCenter(MakeStringView(L"Stride")),
        Fmt::AlignCenter(MakeStringView(L"Max")),

        Fmt::AlignCenter(MakeStringView(L"Size")),
        Fmt::AlignCenter(MakeStringView(L"Peak"))
        );
    Format(oss, fmtFooter,
        Fmt::AlignCenter(MakeStringView(L"Wasted")),
        Fmt::AlignCenter(MakeStringView(L"Peak"))
        );

    oss << Eol;

    STACKLOCAL_WTEXTWRITER(tmp, 256);

    for (const FMemoryTracking* data : datas) {
        Assert(data);

        const FMemoryTracking::FSnapshot usr = data->User();
        const FMemoryTracking::FSnapshot sys = data->System();

        if (usr.AccumulatedAllocs == 0 &&
            sys.AccumulatedAllocs == 0 )
            continue; // skip domains that never allocated

        if (data->Parent() == nullptr)
            oss << hr << Eol;

        tmp.Reset();
        const FStringView name = MakeCStringView(data->Name());

        Format(tmp, L"{0}{1}", Fmt::Repeat(L' ', data->Level()), name);
        Format(oss, fmtTitle, tmp.Written());
        Format(oss, fmtSnapshot,
            Fmt::CountOfElements(usr.NumAllocs),
            Fmt::CountOfElements(usr.PeakAllocs),
            Fmt::SizeInBytes(usr.MinSize),
            Fmt::SizeInBytes(usr.MaxSize),
            Fmt::SizeInBytes(usr.TotalSize),
            Fmt::SizeInBytes(usr.PeakSize)
            );
        Format(oss, fmtSnapshot,
            Fmt::CountOfElements(sys.NumAllocs),
            Fmt::CountOfElements(sys.PeakAllocs),
            Fmt::SizeInBytes(sys.MinSize),
            Fmt::SizeInBytes(sys.MaxSize),
            Fmt::SizeInBytes(sys.TotalSize),
            Fmt::SizeInBytes(sys.PeakSize)
            );
        Format(oss, fmtFooter,
            Fmt::SizeInBytes(sys.TotalSize - usr.TotalSize),
            Fmt::SizeInBytes(sys.PeakSize - usr.PeakSize)
            );

        oss << Eol;
    }

    oss << Eol;
}
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
void FetchAllTrackingDataSorted_(FTrackingDataRegistry_::FMemoryDomainsList* pDatas) {
    FTrackingDataRegistry_::Get().FetchDatas(MakeAppendable(*pDatas));

    std::stable_sort(std::begin(*pDatas), std::end(*pDatas),
        [](const FMemoryTracking* lhs, const FMemoryTracking* rhs) {
            bool less = (lhs->Level() < rhs->Level());

            while (lhs->Level() > rhs->Level()) lhs = lhs->Parent();
            while (rhs->Level() > lhs->Level()) rhs = rhs->Parent();

            for (; !!lhs && !!rhs; lhs = lhs->Parent(), rhs = rhs->Parent()) {
                const int cmp = Compare(
                    MakeCStringView(lhs->Name()),
                    MakeCStringView(rhs->Name()) );

                if (cmp < 0)
                    less = true;
                else if (cmp > 0)
                    less = false;
            }

            return less;
        });
}
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Char>
void DumpTrackingDataFullName_(TBasicTextWriter<_Char>& oss, const FMemoryTracking& data) {
    if (data.Parent()) {
        DumpTrackingDataFullName_(oss, *data.Parent());
        oss << Fmt::Div;
    }

    oss << MakeCStringView(data.Name());
}
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool AllTrackingData(void* user, bool (*each)(void*, TMemoryView<const FMemoryTracking* const>)) NOEXCEPT {
#if USE_PPE_MEMORYDOMAINS
    Assert(each);
    FTrackingDataRegistry_::FMemoryDomainsList datas;
    FetchAllTrackingDataSorted_(&datas);
    return each(user, datas.MakeView());
#else
    Unused(user);
    Unused(each);
    return false;
#endif
}
//----------------------------------------------------------------------------
void RegisterTrackingData(FMemoryTracking *pTrackingData) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().Register(pTrackingData);
#else
    Unused(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void UnregisterTrackingData(FMemoryTracking *pTrackingData) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().Unregister(pTrackingData);
#else
    Unused(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void ReportAllocationFragmentation(FWTextWriter& oss) {
#if USE_PPE_FINAL_RELEASE
    Unused(oss);
#else
    oss << L"Reporting allocator memory info:" << Eol;
    FMallocDebug::DumpMemoryInfo(oss);
#endif
}
//----------------------------------------------------------------------------
void ReportAllocationHistogram(FWTextWriter& oss) {
#if USE_PPE_MEMORYDOMAINS && USE_PPE_LOGGER
    TMemoryView<const size_t> classes;
    TMemoryView<const i64> allocations, totalBytes;
    if (not FMallocDebug::FetchAllocationHistogram(&classes, &allocations, &totalBytes))
        return;

    Assert(classes.size() == allocations.size());

    i64 totalCount = 0;
    i64 maxCount = 0;
    for (i64 count : allocations) {
        totalCount += count;
        maxCount = Max(maxCount, count);
    }

    const auto distribution = [](i64 sz) -> float {
        return (std::log(std::powf(static_cast<float>(sz), 2.0) + 1.f) - std::log(1.f));
    };

    const float distributionScale = distribution(maxCount);

    const auto hr = Fmt::Repeat(L'-', 175);

    oss << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << L"  Report allocations size histogram" << Eol
        << hr << Eol;

    static i64 GPrevAllocations[MemoryDomainsMaxCount] = { 0 }; // beware this is not thread safe
    AssertRelease(lengthof(GPrevAllocations) >= classes.size());

    constexpr float width = 115;
    forrange(i, 0, classes.size()) {
        if (0 == classes[i]) continue;

        if (0 == allocations[i]) {
            Format(oss, L" #{0:#2} | {1:9} | {2:9} | {3:5}% |",
                i,
                Fmt::SizeInBytes(classes[i]),
                Fmt::SizeInBytes(checked_cast<u64>(totalBytes[i])),
                0.f );
        }
        else {
            const auto delta = (allocations[i] - GPrevAllocations[i]);
            Format(oss, L" #{0:#2} | {1:9} | {2:9} | {3:5}% |{4}> {5} +{6}",
                i,
                Fmt::SizeInBytes(classes[i]),
                Fmt::SizeInBytes(checked_cast<u64>(totalBytes[i])),
                100 * float(allocations[i]) / totalCount,
                Fmt::Repeat(delta ? L'=' : L'-', size_t(std::round(Min(width, width * distribution(allocations[i]) / distributionScale)))),
                Fmt::CountOfElements(checked_cast<u64>(allocations[i])),
                delta );
        }

        oss << Eol;

        GPrevAllocations[i] = allocations[i];
    }
#else
    Unused(oss);
#endif
}
//----------------------------------------------------------------------------
void ReportAllTrackingData(FWTextWriter* optional/* = nullptr */)  {
#if !USE_PPE_LOGGER
    if (not optional)
        return;
#endif
#if USE_PPE_MEMORYDOMAINS
    FTrackingDataRegistry_::FMemoryDomainsList datas;
    FetchAllTrackingDataSorted_(&datas);

    FWStringBuilder sb;
    FWTextWriter& oss = (optional ? *optional : sb);

#if USE_PPE_LOGGER
    auto flushLogIFN = [optional, &sb]() {
        if (not optional) {
            FLogger::Log(LOG_MAKESITE(MemoryDomain, Info), sb.Written());
            sb.clear();
        }
    };
#else
    CONSTEXPR auto flushLogIFN = []() CONSTEXPR {};
#endif

    FCurrentProcess::Get().DumpPhysicalMemory(oss);
    flushLogIFN();

    ReportTrackingDatas_(oss, L"Memory domains", datas.MakeView());
    flushLogIFN();

    ReportAllocationHistogram(oss);
    flushLogIFN();

    ReportAllocationFragmentation(oss);
    flushLogIFN();

    if (not optional)
        FLUSH_LOG();

#else
    Unused(optional);
#endif
}
//----------------------------------------------------------------------------
void ReportCsvTrackingData(FTextWriter* optional/* = nullptr */) {
#if USE_PPE_MEMORYDOMAINS
    FTrackingDataRegistry_::FMemoryDomainsList datas;
    FetchAllTrackingDataSorted_(&datas);

    FStringBuilder sb;
    FTextWriter& oss = (optional ? *optional : sb);

    oss << "sep=;" << Eol; // for Excel
    oss << "Level;Name;Segment;NumAllocs;MinSize;MaxSize;TotalSize;PeakAllocs;PeakSize;AccumulatedAllocs;AccumulatedSize" << Eol;

    for (const FMemoryTracking* data : datas) {
        oss << data->Level() << Fmt::SemiColon;
        DumpTrackingDataFullName_(oss, *data);

        const FMemoryTracking::FSnapshot usr = data->User();
        oss << ";USR;"
            << usr.NumAllocs << Fmt::SemiColon
            << usr.MinSize << Fmt::SemiColon
            << usr.MaxSize << Fmt::SemiColon
            << usr.TotalSize << Fmt::SemiColon
            << usr.PeakAllocs << Fmt::SemiColon
            << usr.PeakSize << Fmt::SemiColon
            << usr.AccumulatedAllocs << Fmt::SemiColon
            << usr.AccumulatedSize << Eol;

        oss << data->Level() << Fmt::SemiColon;
        DumpTrackingDataFullName_(oss, *data);

        const FMemoryTracking::FSnapshot sys = data->System();
        oss << ";SYS;"
            << sys.NumAllocs << Fmt::SemiColon
            << sys.MinSize << Fmt::SemiColon
            << sys.MaxSize << Fmt::SemiColon
            << sys.TotalSize << Fmt::SemiColon
            << sys.PeakAllocs << Fmt::SemiColon
            << sys.PeakSize << Fmt::SemiColon
            << sys.AccumulatedAllocs << Fmt::SemiColon
            << sys.AccumulatedSize << Eol;
    }

#   if USE_PPE_LOGGER
    if (not optional) {
        FLogger::Log(LOG_MAKESITE(MemoryDomain, Info), ToWString(sb.Written()) );
    }
#   endif

#else
    Unused(optional);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
