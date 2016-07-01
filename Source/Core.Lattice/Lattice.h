#pragma once

#include "Core.Lattice/Lattice.h"

// TODO: http://www.wazim.com/Collada_Tutorial_1.htm

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LatticeStartup {
public:
    static void Start();
    static void Shutdown();

    static void Clear();
    static void ClearAll_UnusedMemory();

    LatticeStartup() { Start(); }
    ~LatticeStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
