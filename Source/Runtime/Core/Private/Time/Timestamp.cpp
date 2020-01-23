#include "stdafx.h"

#include "Time/Timestamp.h"

#include "HAL/PlatformTime.h"
#include "Memory/HashFunctions.h"
#include "Time/DateTime.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDateTime FTimestamp::ToDateTime() const {
    return FDateTime::FromLocalTime(*this);
}
//----------------------------------------------------------------------------
FDateTime FTimestamp::ToDateTimeUTC() const {
    return FDateTime::FromTimeUTC(*this);
}
//----------------------------------------------------------------------------
size_t FTimestamp::HashValue() const {
    return hash_as_pod(_value);
}
//----------------------------------------------------------------------------
FTimestamp FTimestamp::Now() {
    return FTimestamp{ FPlatformTime::Timestamp() };
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FTimestamp& t) {
    return oss << t.ToDateTime();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FTimestamp& t) {
    return oss << t.ToDateTime();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
