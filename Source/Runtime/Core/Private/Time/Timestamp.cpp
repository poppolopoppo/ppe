#include "stdafx.h"

#include "Time/Timestamp.h"

#include "Time/DateTime.h"
#include "Memory/HashFunctions.h"

#include <time.h>

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
#if defined(PLATFORM_WINDOWS)
    STATIC_ASSERT(sizeof(::__time64_t) == sizeof(value_type));
    FTimestamp t;
    ::_time64(reinterpret_cast<::__time64_t*>(&t._value));
    return t;
#elif defined(PLATFORM_LINUX)
    ::time_t t;
    ::time(&t);
    return FTimestamp{ t };
#else
#   error "unsupported platform"
#endif
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
