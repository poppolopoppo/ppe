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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
