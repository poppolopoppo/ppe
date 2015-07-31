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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LastErrorException::LastErrorException()
:   LastErrorException(GetLastError()) {}
//----------------------------------------------------------------------------
LastErrorException::LastErrorException(long errorCode)
:   std::exception(GetLastErrorToString(errorCode).c_str())
,   _errorCode(errorCode) {}
//----------------------------------------------------------------------------
LastErrorException::~LastErrorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
