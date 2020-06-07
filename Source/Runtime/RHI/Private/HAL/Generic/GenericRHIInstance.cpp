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
bool FGenericInstance::GEnableHDR = true;
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FGenericInstance::GEnableDebug =
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

    GEnableHDR |= process.HasArgument(L"-RHIHdr");
    GEnableHDR &= !process.HasArgument(L"-RHINoHdr");

#if USE_PPE_RHIDEBUG
    GEnableDebug |= process.HasArgument(L"-RHIDebug");
    GEnableDebug &= !process.HasArgument(L"-RHINoDebug");
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
