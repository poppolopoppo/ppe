#include "stdafx.h"

#include "Lattice.h"
#include "Lattice_fwd.h"

#include "Collada.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

PRAGMA_INITSEG_LIB

namespace Core {
namespace Lattice {
POOL_TAG_DEF(Lattice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLatticeModule::Start() {
    CORE_MODULE_START(Lattice);

    POOL_TAG(Lattice)::Start();

    FCollada::Start();
}
//----------------------------------------------------------------------------
void FLatticeModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Lattice);

    FCollada::Shutdown();

    POOL_TAG(Lattice)::Shutdown();
}
//----------------------------------------------------------------------------
void FLatticeModule::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Lattice);

    POOL_TAG(Lattice)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
