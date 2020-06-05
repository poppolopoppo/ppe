#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FGenericDevice : Meta::FNonCopyable {
public: // must be defined by every RHI:
    FGenericDevice() = default;
    virtual ~FGenericDevice() = default;

public: // shared by each device
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
