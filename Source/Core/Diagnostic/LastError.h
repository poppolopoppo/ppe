#pragma once

#include "Core/Core.h"

#include "Core/Diagnostic/Exception.h"
#include "Core/IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString GetLastErrorToString(long errorCode);
//----------------------------------------------------------------------------
FWString GetLastErrorToWString(long errorCode);
//----------------------------------------------------------------------------
class FLastErrorException : public FException {
public:
    FLastErrorException();
    FLastErrorException(long errorCode);
    virtual ~FLastErrorException();

    long ErrorCode() const { return _errorCode; }
private:
    long _errorCode;
};
//----------------------------------------------------------------------------
#if defined(USE_DEBUG_LOGGER) && defined(PLATFORM_WINDOWS)
#   define LOG_LAST_ERROR(_Context) do { \
        const long lastErrorCode_ = GetLastError(); \
        LOG(Error, L"[" _Context "] LastError : {0} = {1}", \
            lastErrorCode_, ::Core::GetLastErrorToWString(lastErrorCode_)); \
    } while (0)
#else
#   define LOG_LAST_ERROR(_Context) NOOP()
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
