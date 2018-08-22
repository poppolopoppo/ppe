#pragma once

#include "Core.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

#include "Diagnostic/Exception.h"
#include "Diagnostic/Logger.h"
#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLastError {
    long Code;
    FLastError();
    FLastError(long code) : Code(code) {}
};
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FLastError& error);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FLastError& error);
//----------------------------------------------------------------------------
class PPE_CORE_API FLastErrorException : public FException {
public:
    FLastErrorException(const char* what);
    FLastErrorException(const char* what, long errorCode);
    virtual ~FLastErrorException();

    const FLastError& ErrorCode() const { return _errorCode; }

private:
    FLastError _errorCode;
};
//----------------------------------------------------------------------------
#if defined(USE_DEBUG_LOGGER) && defined(PLATFORM_WINDOWS)
#   define LOG_LASTERROR(_CATEGORY, _CONTEXT) \
        LOG(_CATEGORY, Error, _CONTEXT " failed, last error : {0}", ::PPE::FLastError())
#   define CLOG_LASTERROR(_CONDITION, _CATEGORY, _CONTEXT) \
        CLOG(_CONDITION, _CATEGORY, Error, _CONTEXT " failed, last error : {0}", ::PPE::FLastError())
#else
#   define LOG_LASTERROR(_CATEGORY, _CONTEXT) NOOP()
#   define CLOG_LASTERROR(_CONDITION, _CATEGORY, _CONTEXT) NOOP()
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
