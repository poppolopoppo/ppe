#include "stdafx.h"

#include "Timestamp.h"

#include "DateTime.h"

#include <time.h>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DateTime Timestamp::ToDateTime() const {
    return DateTime::FromLocalTime(*this);
}
//----------------------------------------------------------------------------
DateTime Timestamp::ToDateTimeUTC() const {
    return DateTime::FromTimeUTC(*this);
}
//----------------------------------------------------------------------------
Timestamp Timestamp::Now() {
    STATIC_ASSERT(sizeof(::__time64_t) == sizeof(value_type));
    Timestamp t;
    ::_time64(reinterpret_cast<::__time64_t*>(&t._value));
    return t;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core