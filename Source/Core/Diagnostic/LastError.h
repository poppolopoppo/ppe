#pragma once

#include "Core/Core.h"

#include "Core/Diagnostic/Exception.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/String_fwd.h"
#include "Core/IO/TextWriter_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API FString GetLastErrorToString(long errorCode);
CORE_API FWString GetLastErrorToWString(long errorCode);
//----------------------------------------------------------------------------
struct CORE_API FLastError {
    long Code;
    FLastError();
    FLastError(long code) : Code(code) {}
};
CORE_API FTextWriter& operator <<(FTextWriter& oss, const FLastError& error);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FLastError& error);
//----------------------------------------------------------------------------
class CORE_API FLastErrorException : public FException {
public:
    FLastErrorException();
    FLastErrorException(long errorCode);
    virtual ~FLastErrorException();

    const FLastError& ErrorCode() const { return _errorCode; }

private:
    FLastError _errorCode;
};
//----------------------------------------------------------------------------
#if defined(USE_DEBUG_LOGGER) && defined(PLATFORM_WINDOWS)
#   define LOG_LAST_ERROR(_CATEGORY, _CONTEXT) \
        LOG(_CATEGORY, Error, _CONTEXT "failed, last error : {0}", ::Core::FLastError())
#else
#   define LOG_LAST_ERROR(_CATEGORY, _CONTEXT) NOOP()
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
