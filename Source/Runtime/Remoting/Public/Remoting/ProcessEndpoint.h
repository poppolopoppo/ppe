#pragma once

#include "Remoting_fwd.h"

#include "Remoting/BaseEndpoint.h"

#include "Container/Vector.h"
#include "IO/String.h"
#include "Time/Timestamp.h"

namespace PPE {
namespace Remoting {
FWD_REFPTR(ProcessAbout);
FWD_REFPTR(ProcessMemoryStats);
FWD_REFPTR(ProcessMemorySnapshot);
FWD_REFPTR(ProcessMemoryTracking);
FWD_REFPTR(ProcessEndpoint);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_REMOTING_API FProcessAbout : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FProcessAbout, RTTI::FMetaObject);

    FString Host;
    FString UserName;
    FString Os;
    FString Executable;
    FString Directory;
    VECTORINSITU(Remoting, FString, 8) Args;

    FString Branch;
    FString Revision;
    FString Family;
    FString Compiler;
    FTimestamp Timestamp;

    FString PlatformDisplayName;
    FString PlatformFullName;
    FString PlatformShortName;
    VECTORINSITU(Remoting, FString, 8) Features;
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FProcessMemoryStats : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FProcessMemoryStats, RTTI::FMetaObject);

    i64 AllocationGranularity;
    i64 CacheLineSize;
    i64 PageSize;
    i64 AvailablePhysical;
    i64 AvailableVirtual;
    i64 UsedPhysical;
    i64 UsedVirtual;
    i64 PeakUsedVirtual;
    i64 PeakUsedPhysical;
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FProcessMemorySnapshot : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FProcessMemorySnapshot, RTTI::FMetaObject);

    i64 NumAllocs;
    i64 MinSize;
    i64 MaxSize;
    i64 TotalSize;
    i64 PeakAllocs;
    i64 PeakSize;
    i64 AccumulatedAllocs;
    i64 AccumulatedSize;
    i64 SmallAllocs;
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FProcessMemoryTracking : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FProcessMemoryTracking, RTTI::FMetaObject);

    FString Domain;
    FString Parent;
    PProcessMemorySnapshot User;
    PProcessMemorySnapshot System;
};
using FProcessMemoryTrackingList = VECTORMINSIZE(Remoting, PProcessMemoryTracking, 32);
//----------------------------------------------------------------------------
class PPE_REMOTING_API FProcessEndpoint final : public FBaseEndpoint {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FProcessEndpoint, FBaseEndpoint);
public:
    FProcessEndpoint();

    /// Platform:
    PProcessAbout About() const;

    /// Memory:
    PProcessMemoryTracking MemoryDomain(const FString& name) const;
    FProcessMemoryTrackingList MemoryDomains() const;
    PProcessMemoryStats MemoryStats() const;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
