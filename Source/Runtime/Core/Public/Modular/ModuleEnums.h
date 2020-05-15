#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"

#include "IO/TextWriter_fwd.h"
#include "Meta/Enum.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EModulePhase {
    Bare,                       // earliest module phase
    System,                     // start all core modules
    Framework,                  // start all generic modules
    Application,                // start all specific modules
    User,                       // latest phase for user modules

    _Max
};
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, EModulePhase phase);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, EModulePhase phase);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EModuleSource {
    Core,                       // modules inside core source code
    Program,                    // program specific modules
    External,                   // external modules, potentially loaded on-the-fly
};
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, EModuleSource source);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, EModuleSource source);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EModuleUsage {
    Runtime    = 0b0000001,     // always needed
    Shipping   = 0b0000011,     // only needed for shipped programs
    Tools      = 0b0000101,     // only needed for tools
    Developer  = 0b0001101,     // always needed, but not shipped
};
ENUM_FLAGS(EModuleUsage);
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, EModuleUsage usage);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, EModuleUsage usage);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
