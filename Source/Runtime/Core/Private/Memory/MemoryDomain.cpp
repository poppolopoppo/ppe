#include "stdafx.h"

#include "Memory/MemoryDomain.h"

#include "Memory/MemoryTracking.h"
#include "Memory/MemoryView.h"

#include "Diagnostic/Logger.h"
#include "Meta/OneTimeInitialize.h"

#if !USE_PPE_FINAL_RELEASE
#   include "Container/IntrusiveList.h"
#   include "Container/Stack.h"
#   include "Diagnostic/CurrentProcess.h"
#   include "Diagnostic/Logger.h"
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
CONSTEXPR size_t MemoryDomainsMaxCount = 128;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryDomain : public FMemoryTracking {
public:
    FMemoryDomain(const char* name, FMemoryTracking* parent)
    :   FMemoryTracking(name, parent) {
        RegisterTrackingData(this);
    }

    ~FMemoryDomain() {
        UnregisterTrackingData(this);
    }

    FMemoryDomain(const FMemoryDomain&) = delete;
    FMemoryDomain& operator =(const FMemoryDomain&) = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MemoryDomain {
    FMemoryTracking& MEMORYDOMAIN_NAME(PooledMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "PooledMemory", nullptr);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(UsedMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "UsedMemory", nullptr);
        return GInstance;
    }
    FMemoryTracking& MEMORYDOMAIN_NAME(ReservedMemory)::TrackingData() {
        ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, "ReservedMemory", nullptr);
        return GInstance;
    }
}
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
// First pass for groups (which are not declared in header)
#if !WITH_PPE_MEMORYDOMAINS_FULL_COLLAPSING
#   define MEMORYDOMAIN_GROUP_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        struct MEMORYDOMAIN_NAME(_Name) { \
            static FMemoryTracking& TrackingData(); \
        }; \
    }
#   include "Memory/MemoryDomain.Definitions-inl.h"
#   undef MEMORYDOMAIN_COLLAPSABLE_IMPL
#   undef MEMORYDOMAIN_GROUP_IMPL
#   undef MEMORYDOMAIN_IMPL
#endif
//----------------------------------------------------------------------------
// Second pass for definition of everything
#if !WITH_PPE_MEMORYDOMAINS_FULL_COLLAPSING
#   define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        FMemoryTracking& MEMORYDOMAIN_NAME(_Name)::TrackingData() { \
            ONE_TIME_INITIALIZE(FMemoryDomain, GInstance, STRINGIZE(_Name), &MEMORYDOMAIN_TRACKING_DATA(_Parent)); \
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
class FTrackingDataRegistry_ {
public:
    static FTrackingDataRegistry_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FTrackingDataRegistry_, GInstance);
        return GInstance;
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
    void FetchDatas(FMemoryDomainsList* plist) const {
        Assert(plist);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        for (FMemoryTracking* node = _domains.Head(); node; node = node->Node.Next)
            plist->Push(node);
    }

private:
    mutable FAtomicSpinLock _barrier;

    INTRUSIVELIST(&FMemoryTracking::Node) _domains;

    FTrackingDataRegistry_() = default;

    FTrackingDataRegistry_(const FTrackingDataRegistry_&) = delete;
    FTrackingDataRegistry_& operator =(const FTrackingDataRegistry_&) = delete;
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

    CONSTEXPR size_t width = 175;
    CONSTEXPR wchar_t fmtTitle[] = L" {0:-30}";
    CONSTEXPR wchar_t fmtSnapshot[] = L"| {0:7} {1:9} | {2:10} {3:10} | {4:11} {5:12}   ";

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

    oss << Eol;

    STACKLOCAL_WTEXTWRITER(tmp, 128);

    for (const FMemoryTracking* data : datas) {
        Assert(data);

        if (data->Parent() == nullptr)
            oss << hr << Eol;

        const FMemoryTracking::FSnapshot usr = data->User();
        const FMemoryTracking::FSnapshot sys = data->System();

        if (sys.AccumulatedAllocs == 0)
            continue; // skip domains that never allocated

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
            Fmt::SizeInBytes(usr.PeakSize) );
        Format(oss, fmtSnapshot,
            Fmt::CountOfElements(sys.NumAllocs),
            Fmt::CountOfElements(sys.PeakAllocs),
            Fmt::SizeInBytes(sys.MinSize),
            Fmt::SizeInBytes(sys.MaxSize),
            Fmt::SizeInBytes(sys.TotalSize),
            Fmt::SizeInBytes(sys.PeakSize) );

        oss << Eol;
    }

    oss << Eol;
}
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
void FetchAllTrackingDataSorted_(FTrackingDataRegistry_::FMemoryDomainsList* pDatas) {
    FTrackingDataRegistry_::Get().FetchDatas(pDatas);

    std::stable_sort(std::begin(*pDatas), std::end(*pDatas),
        [](const FMemoryTracking* lhs, const FMemoryTracking* rhs) {
            bool less = (lhs->Level() < rhs->Level());

            while (lhs->Level() > rhs->Level()) lhs = lhs->Parent();
            while (rhs->Level() > lhs->Level()) rhs = rhs->Parent();

            for (; !!lhs & !!rhs; lhs = lhs->Parent(), rhs = rhs->Parent()) {
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
void RegisterTrackingData(FMemoryTracking *pTrackingData) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().Register(pTrackingData);
#else
    UNUSED(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void UnregisterTrackingData(FMemoryTracking *pTrackingData) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().Unregister(pTrackingData);
#else
    UNUSED(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void ReportAllocationFragmentation(FWTextWriter& oss) {
#if USE_PPE_FINAL_RELEASE
    UNUSED(oss);
#else
    void* vspace;
    size_t numCommited, numReserved, mipSizeInBytes;
    TMemoryView<const u32> mipMasks;

    if (not FMallocDebug::FetchMediumMips(&vspace, &numCommited, &numReserved, &mipSizeInBytes, &mipMasks))
        return;

    CONSTEXPR size_t width = 175;
    const auto hr = Fmt::Repeat(L'-', width);

    oss << hr << Eol
        << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << L"  Report allocation internal fragmentation : "
        << Fmt::SizeInBytes(mipSizeInBytes)
        << L" x "
        << Fmt::CountOfElements(numCommited)
        << L'/'
        << Fmt::CountOfElements(numReserved)
        << L" => "
        << Fmt::SizeInBytes(numCommited * mipSizeInBytes)
        << Eol << hr << Eol;

    Assert_NoAssume(numCommited == mipMasks.size());

    FWString tmp;
    CONSTEXPR const size_t perRow = 16;
    for (size_t r = 0, rows = (numCommited + perRow - 1) / perRow; r < rows; ++r) {
        Format(oss, L"{0}|{1:#3}| ", Fmt::Pointer((u8*)vspace + mipSizeInBytes * r), r * perRow);

        forrange(i, 0, perRow) {
            if (i == 4 || i == 8 || i == 12)
                oss << L' ';

            const size_t mipIndex = (r * perRow + i);
            if (mipIndex < numCommited) {
                tmp = StringFormat(L"{0:#8X}", mipMasks[mipIndex]);
                tmp.gsub('0', '.');
                oss << L' ' << tmp;
            }
        }

        oss << Eol;
    }
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
        return (std::log((float)sz + 1.f) - std::log(1.f));
    };

    const float distributionScale = distribution(maxCount);

    const auto hr = Fmt::Repeat(L'-', 175);

    oss << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << hr << Eol
        << L"  Report allocations size histogram" << Eol
        << hr << Eol;

    static i64 GPrevAllocations[MemoryDomainsMaxCount] = { 0 }; // beware this is not thread safe
    AssertRelease(lengthof(GPrevAllocations) >= classes.size());

    constexpr float width = 115;
    forrange(i, 0, classes.size()) {
        if (0 == classes[i]) continue;

        if (0 == allocations[i])
            Format(oss, L" #{0:#2} | {1:9} | {2:9} | {3:5}% |",
                i,
                Fmt::SizeInBytes(classes[i]),
                Fmt::SizeInBytes(checked_cast<u64>(totalBytes[i])),
                0.f );
        else
            Format(oss, L" #{0:#2} | {1:9} | {2:9} | {3:5}% |{4}> {5} +{6}",
                i,
                Fmt::SizeInBytes(classes[i]),
                Fmt::SizeInBytes(checked_cast<u64>(totalBytes[i])),
                100 * float(allocations[i]) / totalCount,
                Fmt::Repeat(L'=', size_t(std::round(Min(width, width * distribution(allocations[i]) / distributionScale)))),
                Fmt::CountOfElements(checked_cast<u64>(allocations[i])),
                allocations[i] - GPrevAllocations[i] );

        oss << Eol;

        GPrevAllocations[i] = allocations[i];
    }
#else
    UNUSED(oss);
#endif
}
//----------------------------------------------------------------------------
void ReportAllTrackingData(FWTextWriter* optional/* = nullptr */)  {
#if USE_PPE_MEMORYDOMAINS && USE_PPE_LOGGER
    FTrackingDataRegistry_::FMemoryDomainsList datas;
    FetchAllTrackingDataSorted_(&datas);

    FWStringBuilder sb;
    FWTextWriter& oss = (optional ? *optional : sb);

    FCurrentProcess::Get().DumpMemoryStats(oss);

    ReportTrackingDatas_(oss, L"Memory domains", datas.MakeView());
    ReportAllocationHistogram(oss);
    ReportAllocationFragmentation(oss);

    if (not optional) {
        FLogger::Log(
            LOG_CATEGORY_GET(MemoryDomain),
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            sb.Written() );
    }

#else
    UNUSED(optional);
#endif
}
//----------------------------------------------------------------------------
void ReportCsvTrackingData(FTextWriter* optional/* = nullptr */) {
#if USE_PPE_MEMORYDOMAINS
    FTrackingDataRegistry_::FMemoryDomainsList datas;
    FetchAllTrackingDataSorted_(&datas);

    FStringBuilder sb;
    FTextWriter& oss = (optional ? *optional : sb);

    oss << "Name;Segment;NumAllocs;MinSize;MaxSize;TotalSize;PeakAllocs;PeakSize;AccumulatedAllocs;AccumulatedSize" << Eol;

    for (const FMemoryTracking* data : datas) {
        const FMemoryTracking::FSnapshot usr = data->User();
        const FMemoryTracking::FSnapshot sys = data->System();

        DumpTrackingDataFullName_(oss, *data);

        oss << ";USR;"
            << usr.NumAllocs << Fmt::Colon
            << usr.MinSize << Fmt::Colon
            << usr.MaxSize << Fmt::Colon
            << usr.TotalSize << Fmt::Colon
            << usr.PeakAllocs << Fmt::Colon
            << usr.PeakSize << Fmt::Colon
            << usr.AccumulatedAllocs << Fmt::Colon
            << usr.AccumulatedSize << Eol;

        DumpTrackingDataFullName_(oss, *data);

        oss << ";SYS;"
            << sys.NumAllocs << Fmt::Colon
            << sys.MinSize << Fmt::Colon
            << sys.MaxSize << Fmt::Colon
            << sys.TotalSize << Fmt::Colon
            << sys.PeakAllocs << Fmt::Colon
            << sys.PeakSize << Fmt::Colon
            << sys.AccumulatedAllocs << Fmt::Colon
            << sys.AccumulatedSize << Eol;
    }

#   if USE_PPE_LOGGER
    if (not optional) {
        FLogger::Log(
            LOG_CATEGORY_GET(MemoryDomain),
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            ToWString(sb.Written()) );
    }
#   endif

#else
    UNUSED(optional);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
