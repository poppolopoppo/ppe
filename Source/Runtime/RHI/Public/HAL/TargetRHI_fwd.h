#pragma once

#include "RHI_fwd.h"

#define PPE_RHI_MAKEINCLUDE(_BASENAME) \
    STRINGIZE(HAL/TARGET_RHI/CONCAT(TARGET_RHI, _BASENAME).h)

#define PPE_RHI_TARGETALIAS(_PREFIX, _BASENAME) \
    CONCAT3(_PREFIX, TARGET_RHI, _BASENAME)

#define PPE_RHI_MAKEALIAS(_PREFIX, _BASENAME) \
    namespace PPE { namespace RHI { \
        using CONCAT(_PREFIX, _BASENAME) = PPE_RHI_TARGETALIAS(_PREFIX, _BASENAME); \
    }}

#define PPE_RHI_MAKEALIAS_F(_BASENAME) PPE_RHI_MAKEALIAS(F, _BASENAME)
#define PPE_RHI_MAKEALIAS_E(_BASENAME) PPE_RHI_MAKEALIAS(E, _BASENAME)

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETargetRHI;
//----------------------------------------------------------------------------
enum class ERHIFeature;
//----------------------------------------------------------------------------
class PPE_RHI_API ITargetRHI;
//----------------------------------------------------------------------------
PPE_RHI_API TMemoryView<const ITargetRHI* const> AllTargetRHIs();
PPE_RHI_API const ITargetRHI& TargetRHI(ETargetRHI rhi);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE