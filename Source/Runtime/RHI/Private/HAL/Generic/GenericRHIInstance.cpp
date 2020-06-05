#include "stdafx.h"

#include "HAL/Generic/GenericRHIInstance.h"

#if USE_PPE_RHIDEBUG
#    include "Diagnostic/CurrentProcess.h"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FGenericInstance::GHeadless = false;
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FGenericInstance::GDebugEnabled =
#    if USE_PPE_ASSERT
        true;
#else
        false;
#    endif
#endif
//----------------------------------------------------------------------------
void FGenericInstance::ParseOptions() {
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
} //!namespace RHI
} //!namespace PPE
