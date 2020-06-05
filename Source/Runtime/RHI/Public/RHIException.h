#pragma once

#include "RHI_fwd.h"

#include "Diagnostic/Exception.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FRHIException : public FException {
protected:
    FRHIException(const char* what) : FException(what) {}
public:
    virtual ~FRHIException() = default;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
