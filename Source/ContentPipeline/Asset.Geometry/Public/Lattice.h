#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_LATTICE
#   define PPE_LATTICE_API DLL_EXPORT
#else
#   define PPE_LATTICE_API DLL_IMPORT
#endif

// TODO: http://www.wazim.com/Collada_Tutorial_1.htm

namespace PPE {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_LATTICE_API FLatticeModule {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FLatticeModule() { Start(); }
    ~FLatticeModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE
