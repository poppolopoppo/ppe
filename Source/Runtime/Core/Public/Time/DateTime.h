#pragma once

#include "Core.h"

#include "IO/TextWriter_fwd.h"
#include "Memory/HashFunctions.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTimestamp;
class PPE_API FDateTime {
public:
    FDateTime() : FDateTime(0) {}
    explicit FDateTime(u64 ord) { *reinterpret_cast<u64*>(this) = ord; }

    FDateTime(const FDateTime& other) { *reinterpret_cast<u64*>(this) = other.Ord(); }
    FDateTime& operator =(const FDateTime& other) { *reinterpret_cast<u64*>(this) = other.Ord(); return *this; }

    u64 DayOfYear   : 9;
    u64 DayOfWeek   : 3;

    u64 Day         : 5;
    u64 Month       : 4;
    u64 Year        : 26;

    u64 Hours       : 5;
    u64 Minutes     : 6;
    u64 Seconds     : 6;

    u64 Ord() const { return *reinterpret_cast<const u64*>(this); }

    static FDateTime Now();
    static FDateTime FromLocalTime(const FTimestamp& t);
    static FDateTime FromTimeUTC(const FTimestamp& t);

    friend hash_t hash_value(const FDateTime& d) { return hash_as_pod(d.Ord()); }
    friend void swap(FDateTime& lhs, FDateTime& rhs) { std::swap(*reinterpret_cast<u64*>(&lhs), *reinterpret_cast<u64*>(&rhs)); }

    friend bool operator ==(const FDateTime& lhs, const FDateTime& rhs) { return lhs.Ord() == rhs.Ord(); }
    friend bool operator !=(const FDateTime& lhs, const FDateTime& rhs) { return lhs.Ord() != rhs.Ord(); }
};
STATIC_ASSERT(sizeof(FDateTime) == sizeof(u64));
PPE_ASSUME_TYPE_AS_POD(FDateTime)
//----------------------------------------------------------------------------
PPE_API FTextWriter& operator <<(FTextWriter& oss, const FDateTime& d);
PPE_API FWTextWriter& operator <<(FWTextWriter& oss, const FDateTime& d);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
