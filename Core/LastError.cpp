#include "stdafx.h"

#include "LastError.h"

#include "Stream.h"

#ifdef OS_WINDOWS
#   include <comdef.h>
#   include <Windows.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
String GetLastErrorToString(long errorCode) {
#ifdef OS_WINDOWS
    _com_error com(errorCode);

    char tmp[2048];
    {
        OCStrStream oss(tmp);
        oss << com.ErrorMessage();
    }

    return tmp;

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
