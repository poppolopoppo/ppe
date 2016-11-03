#include "stdafx.h"

#include "Status.h"

#include "Core/IO/StringView.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView HttpStatusName(EHttpStatus status) {
    switch (status) {
#define HTTP_STATUSCODE_TOCSTR(_Name, _Value, _Desc) \
    case EHttpStatus::_Name: return STRINGIZE(_Name);
FOREACH_HTTP_STATUSCODE(HTTP_STATUSCODE_TOCSTR)
#undef HTTP_STATUSCODE_TOCSTR
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FStringView HttpStatusCode(EHttpStatus status) {
    switch (status) {
#define HTTP_STATUSCODE_CODECSTR(_Name, _Value, _Desc) \
    case EHttpStatus::_Name: return STRINGIZE(_Value);
FOREACH_HTTP_STATUSCODE(HTTP_STATUSCODE_CODECSTR)
#undef HTTP_STATUSCODE_CODECSTR
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FStringView HttpStatusDescription(EHttpStatus status) {
    switch (status) {
#define HTTP_STATUSCODE_DESCRIPTION(_Name, _Value, _Desc) \
    case EHttpStatus::_Name: return _Desc;
FOREACH_HTTP_STATUSCODE(HTTP_STATUSCODE_DESCRIPTION)
#undef HTTP_STATUSCODE_DESCRIPTION
    }
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
