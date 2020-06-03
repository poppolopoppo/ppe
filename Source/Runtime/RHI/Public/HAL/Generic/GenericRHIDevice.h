#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FGenericRHIDevice : Meta::FNonCopyable {
public: // must be defined by every RHI:
    FGenericRHIDevice() = default;
    virtual ~FGenericRHIDevice() = default;

public: // shared by each device
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
