#pragma once

#include "Network.h"

#include "Exceptions.h"

namespace PPE {
namespace Network {
enum class EHttpStatus;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpException : public FNetworkException {
public:
    FHttpException(EHttpStatus status, const char* what)
        : FNetworkException(what)
        , _status(status) {}

    EHttpStatus Status() const { return _status; }

private:
    EHttpStatus _status;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
