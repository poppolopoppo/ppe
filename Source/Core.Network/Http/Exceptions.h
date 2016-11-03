#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Exceptions.h"

namespace Core {
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
} //!namespace Core
