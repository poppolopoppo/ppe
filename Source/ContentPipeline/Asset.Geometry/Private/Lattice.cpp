// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Lattice.h"
#include "Lattice_fwd.h"

#include "Collada.h"

#include "Allocator/PoolAllocatorTag-impl.h"

PRAGMA_INITSEG_LIB

namespace PPE {
namespace Lattice {
POOL_TAG_DEF(Lattice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLatticeModule::Start() {
    PPE_MODULE_START(Lattice);

    POOL_TAG(Lattice)::Start();

    FCollada::Start();
}
//----------------------------------------------------------------------------
void FLatticeModule::Shutdown() {
    PPE_MODULE_SHUTDOWN(Lattice);

    FCollada::Shutdown();

    POOL_TAG(Lattice)::Shutdown();
}
//----------------------------------------------------------------------------
void FLatticeModule::ClearAll_UnusedMemory() {
    PPE_MODULE_CLEARALL(Lattice);

    POOL_TAG(Lattice)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE
