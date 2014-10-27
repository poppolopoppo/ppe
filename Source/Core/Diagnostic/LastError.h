#pragma once

#include "Core/Core.h"

#include "Core/IO/String.h"

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
