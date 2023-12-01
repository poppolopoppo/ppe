#pragma once

#include "Core.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

#include "Diagnostic/Exception.h"
#include "Diagnostic/Logger.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLastError {
    long Code;
    PPE_CORE_API FLastError();
    FLastError(long code) : Code(code) {}
};
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FLastError& error);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FLastError& error);
//----------------------------------------------------------------------------
class FLastErrorException : public FException {
public:
    PPE_CORE_API explicit FLastErrorException(const char* what);
    PPE_CORE_API FLastErrorException(const char* what, long errorCode);

    const FLastError& ErrorCode() const { return _errorCode; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_CORE_API virtual FTextWriter& Description(FTextWriter& oss) const override final;
#endif

private:
    FLastError _errorCode;
};
//----------------------------------------------------------------------------
#if USE_PPE_LOGGER && defined(PLATFORM_WINDOWS)
#   define PPE_LOG_LASTERROR(_CATEGORY, _CONTEXT) \
        PPE_LOG(_CATEGORY, Error, _CONTEXT " failed, last error : {0}", ::PPE::FLastError())
#   define PPE_CLOG_LASTERROR(_CONDITION, _CATEGORY, _CONTEXT) \
        PPE_CLOG(_CONDITION, _CATEGORY, Error, _CONTEXT " failed, last error : {0}", ::PPE::FLastError())
#else
#   define PPE_LOG_LASTERROR(_CATEGORY, _CONTEXT) NOOP()
#   define PPE_CLOG_LASTERROR(_CONDITION, _CATEGORY, _CONTEXT) Unused(_CONDITION)
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
