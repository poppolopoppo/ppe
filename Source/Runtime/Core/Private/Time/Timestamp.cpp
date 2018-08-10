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
    STATIC_ASSERT(sizeof(::__time64_t) == sizeof(value_type));
    FTimestamp t;
    ::_time64(reinterpret_cast<::__time64_t*>(&t._value));
    return t;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
