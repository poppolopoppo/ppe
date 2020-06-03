#include "stdafx.h"

#include "HAL/Generic/GenericRHIInstance.h"


#if USE_PPE_RHIDEBUG
#        include "Diagnostic/CurrentProcess.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FGenericRHIInstance::GHeadless = false;
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FGenericRHIInstance::GDebugEnabled =
#    if USE_PPE_ASSERT
        true;
#else
        false;
#    endif
#endif
//----------------------------------------------------------------------------
void FGenericRHIInstance::ParseOptions() {
    auto& process = FCurrentProcess::Get();

    GHeadless |= process.HasArgument(L"-RHIHeadless");
    GHeadless &= process.HasArgument(L"-RHINoHeadless");

#if USE_PPE_RHIDEBUG
    GDebugEnabled |= process.HasArgument(L"-RHIDebug");
    GDebugEnabled &= !process.HasArgument(L"-RHINoDebug");
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
