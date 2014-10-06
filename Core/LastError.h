#pragma once

#include "Core.h"

#include "String.h"

#include <exception>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
String GetLastErrorToString(long errorCode);
//----------------------------------------------------------------------------
class LastErrorException : public std::exception {
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
