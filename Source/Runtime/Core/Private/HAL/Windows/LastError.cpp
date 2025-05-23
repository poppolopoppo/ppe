﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Core_fwd.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/LastError.h"

#include "Diagnostic/Logger.h"
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
LOG_CATEGORY(, Exception)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLastError::FLastError() : Code(::GetLastError()) {}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FLastError& error) {
    char buffer[4096];
    if (::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        error.Code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer,
        static_cast<::DWORD>(lengthof(buffer)),
        nullptr)) {
        oss << Chomp(MakeCStringView(buffer)) << " (code=";
    }
    else
    {
        oss << "unknown error (code=";
    }
    return oss << u32(error.Code) << ')';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FLastError& error) {
    wchar_t buffer[4096];
    if (::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        error.Code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer,
        static_cast<::DWORD>(lengthof(buffer)),
        nullptr)) {
        oss << Chomp(MakeCStringView(buffer)) << L" (code=";
    }
    else
    {
        oss << L"unknown error (code=";
    }
    return oss << u32(error.Code) << L')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLastErrorException::FLastErrorException(const char* what)
:   FLastErrorException(what, ::GetLastError())
{}
//----------------------------------------------------------------------------
FLastErrorException::FLastErrorException(const char* what, long errorCode)
:   FException(what)
,   _errorCode(errorCode) {
#if !USE_PPE_FINAL_RELEASE
    PPE_LOG(Exception, Error, "{0}: {1}", MakeCStringView(what), FLastError(_errorCode));
#endif
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FLastErrorException::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What()) << L": "
        << FLastError(_errorCode);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
