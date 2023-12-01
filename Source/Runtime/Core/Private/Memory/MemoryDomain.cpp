// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
    FMemoryDomain(const char* name, FMemoryTracking* parent) NOEXCEPT
    :   FMemoryTracking(name, parent) {
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
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "GpuMemory", nullptr);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(PooledMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "PooledMemory", nullptr);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(ReservedMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "ReservedMemory", nullptr);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(UsedMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "UsedMemory", nullptr);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(VirtualMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "VirtualMemory", nullptr);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(UnaccountedMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "UnaccountedMemory", nullptr);
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
            ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, STRINGIZE(_Name), &parent); \
            return GInstance; \
        } \
    }
#   define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        FMemoryTracking& MEMORYDOMAIN_NAME(_Name)::TrackingData() { \
            auto& parent = const_cast<FMemoryTracking&>(MEMORYDOMAIN_TRACKING_DATA(_Parent)); \
            ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, STRINGIZE(_Name), &parent); \
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

    void RegisterData(FMemoryTracking* pTrackingData) {
        Assert(pTrackingData);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        _domains.PushTail(pTrackingData);

        for (ITrackingDataObserver* pObserver : _observers)
            pObserver->OnRegisterTrackingData(pTrackingData);
    }

    void UnregisterData(FMemoryTracking* pTrackingData) {
        Assert(pTrackingData);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        for (ITrackingDataObserver* pObserver : _observers)
            pObserver->OnUnregisterTrackingData(pTrackingData);

        _domains.Erase(pTrackingData);
    }

    using FMemoryDomainsList = TFixedSizeStack<const FMemoryTracking*, MemoryDomainsMaxCount>;

    void FetchDatas(const TAppendable<const FMemoryTracking*>& plist) const {
        const FAtomicSpinLock::FScope scopeLock(_barrier);

        for (FMemoryTracking* node = _domains.Head(); node; node = node->Node.Next)
            plist.push_back(node);
    }

    void RegisterObserver(ITrackingDataObserver* pObserver) {
        Assert(pObserver);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        _observers.Push(pObserver);

        // replay all tracking data already registered
        for (FMemoryTracking* node = _domains.Head(); node; node = node->Node.Next)
            pObserver->OnRegisterTrackingData(node);
    }

    void UnregisterObserver(ITrackingDataObserver* pObserver) {
        Assert(pObserver);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        const auto it = _observers.Find(pObserver);
        AssertRelease_NoAssume(_observers.end() != it);

        _observers.Erase(it);
    }

private:
    mutable FAtomicSpinLock _barrier;
    INTRUSIVELIST(&FMemoryTracking::Node) _domains;
    TFixedSizeStack<ITrackingDataObserver*, 8> _observers;
};
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
void ReportTrackingDatas_(
    FTextWriter& oss,
    FStringView header,
    const TMemoryView<const FMemoryTracking * const>& datas) {

    if (datas.empty())
        return;

    const FTextWriter::FFormatScope formatScope(oss);

    oss << FTextFormat::Float(FTextFormat::FixedFloat, 2);

    oss << "  Reporting tracking data :" << Eol;

    CONSTEXPR size_t width = 194;
    CONSTEXPR char fmtTitle[] = " {0:/-33}";
    CONSTEXPR char fmtSnapshot[] = "| {0:7} {1:8} | {2:9} {3:9} | {4:10} {5:10}   ";
    CONSTEXPR char fmtFooter[] = "|| {0:10} {1:10}   ";

    const auto hr = Fmt::Repeat('-', width);

    oss << hr << Eol
        << "    " << header << " (" << datas.size() << " elements)" << Eol
        << hr << Eol;

    Format(oss, fmtTitle, "Tracking domain");
    Format(oss, fmtSnapshot,
        Fmt::AlignCenter(MakeStringView("User")),
        Fmt::AlignCenter(MakeStringView("Max")),

        Fmt::AlignCenter(MakeStringView("Stride")),
        Fmt::AlignCenter(MakeStringView("Max")),

        Fmt::AlignCenter(MakeStringView("Size")),
        Fmt::AlignCenter(MakeStringView("Peak"))
        );
    Format(oss, fmtSnapshot,
        Fmt::AlignCenter(MakeStringView("System")),
        Fmt::AlignCenter(MakeStringView("Max")),

        Fmt::AlignCenter(MakeStringView("Stride")),
        Fmt::AlignCenter(MakeStringView("Max")),

        Fmt::AlignCenter(MakeStringView("Size")),
        Fmt::AlignCenter(MakeStringView("Peak"))
        );
    Format(oss, fmtFooter,
        Fmt::AlignCenter(MakeStringView("Wasted")),
        Fmt::AlignCenter(MakeStringView("Peak"))
        );

    oss << Eol;

    STACKLOCAL_TEXTWRITER(tmp, 256);

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

        Format(tmp, "{0}{1}", Fmt::Repeat(' ', data->Level()), name);
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
#if USE_PPE_MEMORYDOMAINS
struct FReportAllTrackingDataToLogger_ {
    ~FReportAllTrackingDataToLogger_() {
        PPE_LOG_FLUSH();
    }
    template <typename _Functor>
    void operator ()(const _Functor& report) const {
        PPE_LOG_DIRECT(MemoryDomain, Info, [&report](FTextWriter& oss) {
            report(oss);
        });
    }
};
template <typename _Char>
struct TReportAllTrackingDataToStream_ {
    TPtrRef<TBasicTextWriter<_Char>> Stream;

    template <typename _Functor>
    void operator ()(const _Functor& report) {
        report(Stream);
    }
};
template <typename _Char, typename _Functor>
static void ReportAllTrackingData_(_Functor&& writer, const FTrackingDataRegistry_::FMemoryDomainsList& datas) {
    writer([&](TBasicTextWriter<_Char>& oss) {
        FCurrentProcess::Get().DumpPhysicalMemory(oss);
    });
    writer([&](TBasicTextWriter<_Char>& oss) {
        ReportTrackingDatas_(oss, "Memory domains", datas);
    });
    writer([&](TBasicTextWriter<_Char>& oss) {
        ReportTrackingDatas_(oss, "Memory domains", datas.MakeView());
    });
    writer([&](TBasicTextWriter<_Char>& oss) {
        ReportAllocationHistogram(oss);
    });
    writer([&](TBasicTextWriter<_Char>& oss) {
        ReportAllocationFragmentation(oss);
    });
}
template <typename _Char>
static void ReportCsvTrackingData_(TBasicTextWriter<_Char>& oss, const FTrackingDataRegistry_::FMemoryDomainsList& datas) {
    oss << STRING_LITERAL(_Char, "sep=;") << Eol; // for Excel
    oss << STRING_LITERAL(_Char, "Level;Name;Segment;NumAllocs;MinSize;MaxSize;TotalSize;PeakAllocs;PeakSize;AccumulatedAllocs;AccumulatedSize") << Eol;

    for (const FMemoryTracking* data : datas) {
        oss << data->Level() << Fmt::SemiColon;
        DumpTrackingDataFullName_(oss, *data);

        const FMemoryTracking::FSnapshot usr = data->User();
        oss << STRING_LITERAL(_Char, ";USR;")
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
        oss << STRING_LITERAL(_Char, ";SYS;")
            << sys.NumAllocs << Fmt::SemiColon
            << sys.MinSize << Fmt::SemiColon
            << sys.MaxSize << Fmt::SemiColon
            << sys.TotalSize << Fmt::SemiColon
            << sys.PeakAllocs << Fmt::SemiColon
            << sys.PeakSize << Fmt::SemiColon
            << sys.AccumulatedAllocs << Fmt::SemiColon
            << sys.AccumulatedSize << Eol;
    }
}
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterTrackingDataObserver(ITrackingDataObserver* pObserver) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().RegisterObserver(pObserver);
#else
    Unused(pObserver);
#endif
}
//----------------------------------------------------------------------------
void UnregisterTrackingDataObserver(ITrackingDataObserver* pObserver) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().UnregisterObserver(pObserver);
#else
    Unused(pObserver);
#endif
}
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
    FTrackingDataRegistry_::Get().RegisterData(pTrackingData);
#else
    Unused(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void UnregisterTrackingData(FMemoryTracking *pTrackingData) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().UnregisterData(pTrackingData);
#else
    Unused(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void ReportAllocationFragmentation(FTextWriter& oss) {
#if USE_PPE_FINAL_RELEASE
    Unused(oss);
#else
    oss << "Reporting allocator memory info:" << Eol;
    FMallocDebug::DumpMemoryInfo(oss);
#endif
}
//----------------------------------------------------------------------------
void ReportAllocationHistogram(FTextWriter& oss) {
#if USE_PPE_MEMORYDOMAINS && USE_PPE_LOGGER
    TMemoryView<const u32> sizeClasses;
    TMemoryView<const FMemoryTracking> bins;
    if (not FMallocDebug::FetchAllocationHistogram(&sizeClasses, &bins))
        return;

    Assert(not sizeClasses.empty());
    Assert_NoAssume(sizeClasses.size() == bins.size());

    i64 totalCount = 0;
    i64 maxCount = 0;
    for (const FMemoryTracking& trackingData : bins) {
        const FMemoryTracking::FSnapshot user = trackingData.User();
        totalCount += user.AccumulatedAllocs;
        maxCount = Max(maxCount, user.AccumulatedAllocs);
    }

    const auto distribution = [bias{ std::log(1.f) }](i64 n) -> float {
        return (std::log(std::pow(static_cast<float>(n), 2.0f) + 1.f) - bias);
    };

    const float distributionScale = distribution(maxCount);

    const auto hr = Fmt::Repeat('-', 175);

    oss << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << "  Report allocations size histogram" << Eol
        << hr << Eol;

    static i64 GPrevAllocations[MemoryDomainsMaxCount] = { 0 }; // beware this is not thread safe
    AssertRelease(lengthof(GPrevAllocations) >= sizeClasses.size());

    constexpr float width = 106;
    forrange(i, 0, sizeClasses.size()) {
        Assert_NoAssume(sizeClasses[i] > 0);

        const FMemoryTracking& trackingData = bins[i];
        const FMemoryTracking::FSnapshot user = trackingData.User();
        const FMemoryTracking::FSnapshot system = trackingData.System();
        const FMemoryTracking::FSnapshot wasted = trackingData.Wasted();

        if (0 == user.AccumulatedAllocs) {
            Format(oss, " #{0:#3} | {1:9} | {2:9} | {3:9} | {4} |",
                i,
                Fmt::SizeInBytes(sizeClasses[i]),
                Fmt::SizeInBytes(checked_cast<u64>(system.AccumulatedSize)),
                Fmt::SizeInBytes(checked_cast<u64>(wasted.AccumulatedSize)),
                Fmt::Percentage(0, 1) );
        }
        else {
            const auto delta = (user.AccumulatedAllocs - GPrevAllocations[i]);
            Format(oss, " #{0:#3} | {1:9} | {2:9} | {3:9} | {4} |{5}> {6} +{7}",
                i,
                Fmt::SizeInBytes(sizeClasses[i]),
                Fmt::SizeInBytes(checked_cast<u64>(system.AccumulatedSize)),
                Fmt::SizeInBytes(checked_cast<u64>(wasted.AccumulatedSize)),
                Fmt::Percentage(user.AccumulatedAllocs, totalCount),
                Fmt::Repeat(delta ? '=' : '-', size_t(std::round(Min(width, width * distribution(user.AccumulatedAllocs) / distributionScale)))),
                Fmt::CountOfElements(checked_cast<u64>(user.AccumulatedAllocs)),
                delta );
        }

        oss << Eol;

        GPrevAllocations[i] = user.AccumulatedAllocs;
    }
#else
    Unused(oss);
#endif
}
//----------------------------------------------------------------------------
void ReportAllTrackingData(FTextWriter* optional/* = nullptr */)  {
#if !USE_PPE_LOGGER
    if (not optional)
        return;
#endif
#if USE_PPE_MEMORYDOMAINS
    FTrackingDataRegistry_::FMemoryDomainsList datas;
    FetchAllTrackingDataSorted_(&datas);

    if (optional)
        ReportAllTrackingData_<char>(TReportAllTrackingDataToStream_<char>{optional}, datas);
    else
        ReportAllTrackingData_<char>(FReportAllTrackingDataToLogger_{}, datas);
#else
    Unused(optional);
#endif
}
//----------------------------------------------------------------------------
void ReportCsvTrackingData(FTextWriter* optional/* = nullptr */) {
#if USE_PPE_MEMORYDOMAINS
    FTrackingDataRegistry_::FMemoryDomainsList datas;
    FetchAllTrackingDataSorted_(&datas);

    if (optional) {
        ReportCsvTrackingData_(*optional, datas);
    }
    else {
        PPE_LOG_DIRECT(MemoryDomain, Info, [&](FTextWriter& oss) {
            ReportCsvTrackingData_(oss, datas);
        });
    }
#else
    Unused(optional);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
