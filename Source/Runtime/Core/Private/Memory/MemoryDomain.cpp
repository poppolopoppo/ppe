#include "stdafx.h"

#include "Memory/MemoryDomain.h"

#include "Memory/MemoryTracking.h"
#include "Memory/MemoryView.h"

#include "Diagnostic/Logger.h"
#include "Meta/OneTimeInitialize.h"

#if !USE_PPE_FINAL_RELEASE
#   include "Container/IntrusiveList.h"
#   include "Container/Stack.h"
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
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterTrackingData(FMemoryTracking *pTrackingData) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().Register(pTrackingData);
#else
    NOOP(pTrackingData);
#endif
}
//----------------------------------------------------------------------------
void UnregisterTrackingData(FMemoryTracking *pTrackingData) {
#if USE_PPE_MEMORYDOMAINS
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    FTrackingDataRegistry_::Get().Unregister(pTrackingData);
#else
    NOOP(pTrackingData);
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
#if USE_PPE_MEMORYDOMAINS && defined(USE_DEBUG_LOGGER)
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
    NOOP(oss);
#endif
}
//----------------------------------------------------------------------------
void ReportAllTrackingData(FWTextWriter* optional/* = nullptr */)  {
#if USE_PPE_MEMORYDOMAINS && defined(USE_DEBUG_LOGGER)
    FTrackingDataRegistry_::FMemoryDomainsList datas;
    FTrackingDataRegistry_::Get().FetchDatas(&datas);

    {
        STACKLOCAL_POD_ARRAY(u32, indices, datas.size());
        TFixedSizeStack<FString, MemoryDomainsMaxCount> fullnames;

        forrange(i, 0, datas.size()) {
            indices[i] = u32(i);
            FString* str = INPLACE_NEW(fullnames.Push_Uninitialized(), FString)();
            str->reserve(128);
            for (const FMemoryTracking* p = datas[i]; p; p = p->Parent()) {
                str->insert(str->begin(), MakeCStringView(p->Name()));
                if (p == &MEMORYDOMAIN_TRACKING_DATA(PooledMemory))
                    str->insert(str->begin(), 'Z'); // always listed last
            }
        }

        std::stable_sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return (Compare(fullnames[i], fullnames[j]) < 0);
        });

        ReindexMemoryView(datas.MakeView(), indices);
    }

    FWStringBuilder sb;
    FWTextWriter& oss = (optional ? *optional : sb);

    ReportTrackingDatas_(oss, L"Memory domains", datas.MakeView());
    ReportAllocationHistogram(oss);
    ReportAllocationFragmentation(oss);

    if (not optional)
        FLogger::Log(
            LOG_CATEGORY_GET(MemoryDomain),
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            sb.Written() );

#else
    NOOP(optional);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
