#pragma once

#include "Core_fwd.h"

#include "Diagnostic/Logger_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_LOG_CATEGORY(PPE_CORE_API, Modular)
//----------------------------------------------------------------------------
enum class EModulePhase;
enum class EModuleSource;
enum class EModuleUsage;
//----------------------------------------------------------------------------
struct FModuleInfo;
class IModuleInterface;
//----------------------------------------------------------------------------
class FModularDomain;
class FModularServices;
//----------------------------------------------------------------------------
using FModuleLoadOrder = int;
//----------------------------------------------------------------------------
PPE_CORE_API void DutyCycleForModules() NOEXCEPT;
PPE_CORE_API void ReleaseMemoryInModules() NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE