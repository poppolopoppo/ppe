#include "stdafx.h"

#include "HAL/Windows/LastError.h"

#ifdef PLATFORM_WINDOWS

#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"

#ifndef UNICODE
#   error "invalid TCHAR format"
#endif

#include "IO/Format.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLastError::FLastError() : Code(::GetLastError()) {}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FLastError& error) {
#ifdef PLATFORM_WINDOWS
    char buffer[4096];
    if (::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        error.Code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer,
        lengthof(buffer),
        nullptr)) {
        oss << MakeCStringView(buffer) << " (code=";
    }
    else
#endif
    {
        oss << "unknown error (code=";
    }
    return oss << u32(error.Code) << ')';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FLastError& error) {
#ifdef PLATFORM_WINDOWS
    wchar_t buffer[4096];
    if (::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        error.Code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer,
        lengthof(buffer),
        nullptr)) {
        oss << MakeCStringView(buffer) << L" (code=";
    }
    else
#endif
    {
        oss << L"unknown error (code=";
    }
    return oss << u32(error.Code) << L')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLastErrorException::FLastErrorException(const char* what)
:   FLastErrorException(what, GetLastError()) {}
//----------------------------------------------------------------------------
FLastErrorException::FLastErrorException(const char* what, long errorCode)
:   FException(what)
,   _errorCode(errorCode) {}
//----------------------------------------------------------------------------
FLastErrorException::~FLastErrorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
