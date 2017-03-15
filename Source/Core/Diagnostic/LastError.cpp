#include "stdafx.h"

#include "LastError.h"

#include "IO/Stream.h"
#include "IO/StringView.h"

#ifdef PLATFORM_WINDOWS

#   ifndef UNICODE
#       error "invalid TCHAR format"
#   endif

#   include <Windows.h>
#   include <comdef.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString GetLastErrorToString(long errorCode) {
#ifdef PLATFORM_WINDOWS
    _com_error com(errorCode);
    return ToString(MakeStringView(com.ErrorMessage(), Meta::noinit_tag()));

#else
    return FString();

#endif
}
//----------------------------------------------------------------------------
FWString GetLastErrorToWString(long errorCode) {
#ifdef PLATFORM_WINDOWS
    _com_error com(errorCode);
    return ToWString(MakeStringView(com.ErrorMessage(), Meta::noinit_tag()));

#else
    return FWString();

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLastErrorException::FLastErrorException()
:   FLastErrorException(GetLastError()) {}
//----------------------------------------------------------------------------
FLastErrorException::FLastErrorException(long errorCode)
:   FException(GetLastErrorToString(errorCode).c_str())
,   _errorCode(errorCode) {}
//----------------------------------------------------------------------------
FLastErrorException::~FLastErrorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
