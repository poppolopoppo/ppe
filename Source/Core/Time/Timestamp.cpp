#include "stdafx.h"

#include "Timestamp.h"

#include "DateTime.h"

#include <time.h>

namespace Core {
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
FTimestamp FTimestamp::Now() {
    STATIC_ASSERT(sizeof(::__time64_t) == sizeof(value_type));
    FTimestamp t;
    ::_time64(reinterpret_cast<::__time64_t*>(&t._value));
    return t;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
