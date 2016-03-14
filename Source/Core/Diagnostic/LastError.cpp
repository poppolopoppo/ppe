#include "stdafx.h"

#include "LastError.h"

#include "IO/Stream.h"

#ifdef OS_WINDOWS

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
String GetLastErrorToString(long errorCode) {
#ifdef OS_WINDOWS
    _com_error com(errorCode);
    return ToString(com.ErrorMessage());

#else
    return String();

#endif
}
//----------------------------------------------------------------------------
WString GetLastErrorToWString(long errorCode) {
#ifdef OS_WINDOWS
    _com_error com(errorCode);
    return ToWString(com.ErrorMessage());

#else
    return WString();

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LastErrorException::LastErrorException()
:   LastErrorException(GetLastError()) {}
//----------------------------------------------------------------------------
LastErrorException::LastErrorException(long errorCode)
:   Exception(GetLastErrorToString(errorCode).c_str())
,   _errorCode(errorCode) {}
//----------------------------------------------------------------------------
LastErrorException::~LastErrorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
