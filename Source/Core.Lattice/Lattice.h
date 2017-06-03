#pragma once

#include "Core.Lattice/Lattice.h"

#ifdef EXPORT_CORE_LATTICE
#   define CORE_LATTICE_API DLL_EXPORT
#else
#   define CORE_LATTICE_API DLL_IMPORT
#endif

// TODO: http://www.wazim.com/Collada_Tutorial_1.htm

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_LATTICE_API FLatticeModule {
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
} //!namespace Core
