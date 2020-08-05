#pragma once

#include "Network_fwd.h"

#include "NetworkExceptions.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FHttpException : public FNetworkException {
public:
    FHttpException(EHttpStatus status, const char* what)
        : FNetworkException(what)
        , _status(status) {}

    EHttpStatus Status() const { return _status; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif

private:
    EHttpStatus _status;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
