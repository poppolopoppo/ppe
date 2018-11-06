#include "stdafx.h"

#include "Memory/MemoryDomain.h"

#include "Memory/MemoryTracking.h"
#include "Memory/MemoryView.h"

#include "Diagnostic/Logger.h"
#include "Meta/OneTimeInitialize.h"

#if USE_PPE_MEMORYDOMAINS
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
constexpr size_t MemoryDomainsMaxCount = 128;
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
        Assert(nullptr == pTrackingData->Prev());
        Assert(nullptr == pTrackingData->Next());

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        if (_head) {
            pTrackingData->SetNext(_head);
            _head->SetPrev(pTrackingData);
        }

        _head = pTrackingData;
    }

    void Unregister(FMemoryTracking* pTrackingData) {
        Assert(pTrackingData);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        if (pTrackingData->Prev()) {
            Assert(pTrackingData != _head);
            pTrackingData->Prev()->SetNext(pTrackingData->Next());
        }
        else {
            Assert(pTrackingData == _head);
            _head = pTrackingData->Next();
        }

        if (pTrackingData->Next())
            pTrackingData->Next()->SetPrev(pTrackingData->Prev());
    }

    using FMemoryDomainsList = TFixedSizeStack<const FMemoryTracking*, MemoryDomainsMaxCount>;
    void FetchDatas(FMemoryDomainsList* plist) const {
        Assert(plist);

        const FAtomicSpinLock::FScope scopeLock(_barrier);

        for (FMemoryTracking* node = _head; node; node = node->Next())
            plist->Push(node);
    }

private:
    mutable FAtomicSpinLock _barrier;
    FMemoryTracking* _head;

    FTrackingDataRegistry_() : _head(nullptr) {}

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

    oss << L"reporting tracking data :" << Eol;

    const size_t width = 128;
    const wchar_t fmt[] = L" {0:-33}| {1:8} {2:10} | {3:8} {4:11} | {5:9} {6:11} | {7:10} {8:11}\n";

    oss << Fmt::Repeat(L'-', width) << Eol
        << L"    " << header << L" (" << datas.size() << L" elements)" << Eol
        << Fmt::Repeat(L'-', width) << Eol;

    Format(oss, fmt, L"Tracking domain",
        L"Block",   L"Max",
        L"Alloc",   L"Max",
        L"Stride",  L"Max",
        L"Total",   L"Max" );

    oss << Fmt::Repeat(L'-', width) << Eol;

    STACKLOCAL_WTEXTWRITER(tmp, 128);

    for (const FMemoryTracking* data : datas) {
        Assert(data);
        tmp.Reset();
        const FStringView name = MakeCStringView(data->Name());
        Format(tmp, L"{0}{1}", Fmt::Repeat(L' ', 2 * data->Level()), name);
        Format(oss, fmt,
            tmp.Written(),
            Fmt::FCountOfElements{ data->BlockCount() },
            Fmt::FCountOfElements{ data->MaxBlockCount() },
            Fmt::FCountOfElements{ data->AllocationCount() },
            Fmt::FCountOfElements{ data->MaxAllocationCount() },
            Fmt::FSizeInBytes{ Min(data->MaxStrideInBytes(), data->MinStrideInBytes()) },
            Fmt::FSizeInBytes{ data->MaxStrideInBytes() },
            Fmt::FSizeInBytes{ data->TotalSizeInBytes() },
            Fmt::FSizeInBytes{ data->MaxTotalSizeInBytes() });
    }

    oss << Fmt::Repeat(L'-', width) << Eol;
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
void ReportAllocationHistogram(FWTextWriter& oss) {
#if USE_PPE_MEMORYDOMAINS && defined(USE_DEBUG_LOGGER)
    TMemoryView<const size_t> classes;
    TMemoryView<const i64> allocations, totalBytes;
    if (not FetchMemoryAllocationHistogram(&classes, &allocations, &totalBytes))
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

    oss << FTextFormat::Float(FTextFormat::FixedFloat, 2);
    oss << L"report allocations size histogram" << Eol;

    static i64 GPrevAllocations[MemoryDomainsMaxCount] = { 0 }; // beware this is not thread safe
    AssertRelease(lengthof(GPrevAllocations) >= classes.size());

    constexpr float width = 80;
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

    if (not optional)
        FLogger::Log(
            GLogCategory_MemoryDomain,
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            sb.ToString() );
#else
    NOOP(optional);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
