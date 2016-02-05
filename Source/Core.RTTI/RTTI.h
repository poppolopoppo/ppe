#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTIProperties.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class RTTIStartup {
public:
    static void Start();
    static void Shutdown();

    static void Clear();
    static void ClearAll_UnusedMemory();

    RTTIStartup() { Start(); }
    ~RTTIStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
