// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Remoting/ProcessEndpoint.h"

#include "RemotingModule.h"
#include "Diagnostic/CurrentProcess.h"

#include "Json/Json.h"

#include "RTTI/Macros-impl.h"
#include "RTTI/OpaqueData.h"

#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "Memory/RefPtr.h"
#include "Modular/ModuleInfo.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Assign_(FProcessMemorySnapshot* dst, const FMemoryTracking::FSnapshot& src) {
    dst->NumAllocs = src.NumAllocs;
    dst->MinSize = src.MinSize;
    dst->MaxSize = src.MaxSize;
    dst->TotalSize = src.TotalSize;
    dst->PeakAllocs = src.PeakAllocs;
    dst->PeakSize = src.PeakSize;
    dst->AccumulatedAllocs = src.AccumulatedAllocs;
    dst->AccumulatedSize = src.AccumulatedSize;
    dst->SmallAllocs = src.SmallAllocs;
}
//----------------------------------------------------------------------------
static void Assign_(FProcessMemoryTracking* dst, const FMemoryTracking& src) {
    if (dst->Domain.empty())
        dst->Domain.assign(MakeCStringView(src.Name()));
    else
        Assert_NoAssume(MakeCStringView(src.Name()) == dst->Domain);

    if (src.Parent())
        dst->Parent.assign(MakeCStringView(src.Parent()->Name()));

    if (not dst->User)
        dst->User = NEW_RTTI(FProcessMemorySnapshot);
    Assign_(dst->User.get(), src.User());

    if (not dst->System)
        dst->System = NEW_RTTI(FProcessMemorySnapshot);
    Assign_(dst->System.get(), src.System());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FProcessAbout, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Host)
RTTI_PROPERTY_PUBLIC_FIELD(UserName)
RTTI_PROPERTY_PUBLIC_FIELD(Os)
RTTI_PROPERTY_PUBLIC_FIELD(Executable)
RTTI_PROPERTY_PUBLIC_FIELD(Directory)
RTTI_PROPERTY_PUBLIC_FIELD(Args)
RTTI_PROPERTY_PUBLIC_FIELD(Branch)
RTTI_PROPERTY_PUBLIC_FIELD(Revision)
RTTI_PROPERTY_PUBLIC_FIELD(Family)
RTTI_PROPERTY_PUBLIC_FIELD(Compiler)
RTTI_PROPERTY_PUBLIC_FIELD(Timestamp)
RTTI_PROPERTY_PUBLIC_FIELD(PlatformDisplayName)
RTTI_PROPERTY_PUBLIC_FIELD(PlatformFullName)
RTTI_PROPERTY_PUBLIC_FIELD(PlatformShortName)
RTTI_PROPERTY_PUBLIC_FIELD(Features)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FProcessMemoryStats, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(AllocationGranularity)
RTTI_PROPERTY_PUBLIC_FIELD(CacheLineSize)
RTTI_PROPERTY_PUBLIC_FIELD(PageSize)
RTTI_PROPERTY_PUBLIC_FIELD(AvailablePhysical)
RTTI_PROPERTY_PUBLIC_FIELD(AvailableVirtual)
RTTI_PROPERTY_PUBLIC_FIELD(UsedPhysical)
RTTI_PROPERTY_PUBLIC_FIELD(UsedVirtual)
RTTI_PROPERTY_PUBLIC_FIELD(PeakUsedVirtual)
RTTI_PROPERTY_PUBLIC_FIELD(PeakUsedPhysical)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FProcessMemorySnapshot, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(NumAllocs)
RTTI_PROPERTY_PUBLIC_FIELD(MinSize)
RTTI_PROPERTY_PUBLIC_FIELD(MaxSize)
RTTI_PROPERTY_PUBLIC_FIELD(TotalSize)
RTTI_PROPERTY_PUBLIC_FIELD(PeakAllocs)
RTTI_PROPERTY_PUBLIC_FIELD(PeakSize)
RTTI_PROPERTY_PUBLIC_FIELD(AccumulatedAllocs)
RTTI_PROPERTY_PUBLIC_FIELD(AccumulatedSize)
RTTI_PROPERTY_PUBLIC_FIELD(SmallAllocs)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FProcessMemoryTracking, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Domain)
RTTI_PROPERTY_PUBLIC_FIELD(Parent)
RTTI_PROPERTY_PUBLIC_FIELD(User)
RTTI_PROPERTY_PUBLIC_FIELD(System)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FProcessEndpoint, Concrete)
RTTI_FUNCTION_FACET(About, (), FOperationFacet::Get("about", {}))
RTTI_FUNCTION_FACET(MemoryDomain, (name), FOperationFacet::Get("memory/domain", { "{name}" }))
RTTI_FUNCTION_FACET(MemoryDomains, (), FOperationFacet::Get("memory/domains", {}))
RTTI_FUNCTION_FACET(MemoryStats, (), FOperationFacet::Get("memory/stats", {}))
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FProcessEndpoint::FProcessEndpoint()
:   FBaseEndpoint("/process"){

}
//----------------------------------------------------------------------------
PProcessAbout FProcessEndpoint::About() const {
    const auto& proc = FCurrentProcess::Get();

    PProcessAbout about = NEW_RTTI(FProcessAbout);

    about->Host = FPlatformMisc::MachineName();
    about->UserName = FPlatformMisc::UserName();
    about->Os = FPlatformMisc::OSName();
    about->Executable = ToString(proc.ExecutableName());
    about->Args = proc.Args().Map([](const FWStringView& wstr) {
        return ToString(wstr);
    });
    about->Directory = ToString(proc.Directory());

    const FBuildVersion& build = FRemotingModule::StaticInfo.BuildVersion;

    about->Branch = build.Branch;
    about->Revision = build.Revision;
    about->Family = build.Family;
    about->Compiler = build.Compiler;
    about->Timestamp = build.Timestamp;

    const ITargetPlaftorm& platform = CurrentPlatform();
    if (platform.SupportsFeature(EPlatformFeature::Client))
        about->Features.emplace_back("client");
    if (platform.SupportsFeature(EPlatformFeature::Server))
        about->Features.emplace_back("cerver");
    if (platform.SupportsFeature(EPlatformFeature::Editor))
        about->Features.emplace_back("editor");
    if (platform.SupportsFeature(EPlatformFeature::DataGeneration))
        about->Features.emplace_back("data_generation");
    if (platform.SupportsFeature(EPlatformFeature::HighQuality))
        about->Features.emplace_back("high_quality");
    if (platform.SupportsFeature(EPlatformFeature::CookedData))
        about->Features.emplace_back("cooked_data");

    about->PlatformFullName = platform.FullName();
    about->PlatformDisplayName = platform.DisplayName();
    about->PlatformShortName = platform.ShortName();

    return about;
}
//----------------------------------------------------------------------------
PProcessMemoryTracking FProcessEndpoint::MemoryDomain(const FString& name) const {
    auto result = NEW_RTTI(FProcessMemoryTracking);
    result->Domain = name;

    ForeachTrackingDataUnsorted(result.get(), [](void* user, const FMemoryTracking& domain) {
        auto* result = static_cast<FProcessMemoryTracking*>(user);
        if (EqualsI(MakeCStringView(domain.Name()), result->Domain)) {
            Assign_(result, domain);
            return true;
        }
        return false;
    });

    return result;
}
//----------------------------------------------------------------------------
FProcessMemoryTrackingList FProcessEndpoint::MemoryDomains() const {
    FProcessMemoryTrackingList results;

    AllTrackingDataSorted(&results, [](void* user, TMemoryView<const FMemoryTracking* const> domains) {
        auto& list = *static_cast<FProcessMemoryTrackingList*>(user);

        list.reserve(domains.size());
        list.append(
            domains.Map([](const FMemoryTracking* tracking) -> PProcessMemoryTracking {
                auto result = NEW_RTTI(FProcessMemoryTracking);
                Assign_(result.get(), *tracking);
                return result;
            }));

        return true;
    });

    return results;
}
//----------------------------------------------------------------------------
PProcessMemoryStats FProcessEndpoint::MemoryStats() const {
    const auto infos = FPlatformMemory::Constants();
    const auto stats = FPlatformMemory::Stats();

    auto result = NEW_RTTI(FProcessMemoryStats);
    result->AllocationGranularity = checked_cast<i64>(infos.AllocationGranularity);
    result->CacheLineSize = checked_cast<i64>(infos.CacheLineSize);
    result->PageSize = checked_cast<i64>(infos.PageSize);
    result->AvailablePhysical = checked_cast<i64>(stats.AvailablePhysical);
    result->AvailableVirtual = checked_cast<i64>(stats.AvailableVirtual);
    result->UsedPhysical = checked_cast<i64>(stats.UsedPhysical);
    result->UsedVirtual = checked_cast<i64>(stats.UsedVirtual);
    result->PeakUsedPhysical = checked_cast<i64>(stats.PeakUsedPhysical);
    result->PeakUsedVirtual = checked_cast<i64>(stats.PeakUsedVirtual);

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
