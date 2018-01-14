#include "stdafx.h"

#include "LastError.h"

#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"

#ifdef PLATFORM_WINDOWS

#   ifndef UNICODE
#       error "invalid TCHAR format"
#   endif

#   include "IO/Format.h"
#   include "Misc/Platform_Windows.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString GetLastErrorToString(long errorCode) {
#ifdef PLATFORM_WINDOWS
    _com_error com(errorCode);
    //return ToString(MakeStringView(com.ErrorMessage(), Meta::FForceInit{}));
    return StringFormat("{0:#4X}: {1}", u32(errorCode), com.ErrorMessage());

#else
    return FString();

#endif
}
//----------------------------------------------------------------------------
FWString GetLastErrorToWString(long errorCode) {
#ifdef PLATFORM_WINDOWS
    _com_error com(errorCode);
    //return ToWString(MakeStringView(com.ErrorMessage(), Meta::FForceInit{}));
    return StringFormat(L"{0:#4X}: {1}", u32(errorCode), com.ErrorMessage());

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
