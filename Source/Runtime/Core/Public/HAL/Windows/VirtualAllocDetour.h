﻿#pragma once

#include "Core_fwd.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid platform include"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Virtual allocation hooks
//----------------------------------------------------------------------------
struct FVirtualAllocDetour {
    static PPE_CORE_API bool StartHooks();
    static PPE_CORE_API void ShutdownHooks();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE