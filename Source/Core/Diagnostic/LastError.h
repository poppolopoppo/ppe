#pragma once

#include "Core/Core.h"

#include "Core/Diagnostic/Exception.h"
#include "Core/IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
String GetLastErrorToString(long errorCode);
//----------------------------------------------------------------------------
class LastErrorException : public Exception {
public:
    LastErrorException();
    LastErrorException(long errorCode);
    virtual ~LastErrorException();

    long ErrorCode() const { return _errorCode; }
private:
    long _errorCode;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
