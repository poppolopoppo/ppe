#include "stdafx.h"

#include "Lattice.h"
#include "Lattice_fwd.h"

#include "Collada.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#ifdef CPP_VISUALSTUDIO
#   pragma warning(disable: 4073) // initialiseurs plac�s dans la zone d'initialisation d'une biblioth�que
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace Lattice {
POOL_TAG_DEF(Lattice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void LatticeStartup::Start() {
    CORE_MODULE_START(Lattice);

    POOL_TAG(Lattice)::Start();

    FCollada::Start();
}
//----------------------------------------------------------------------------
void LatticeStartup::Shutdown() {
    CORE_MODULE_SHUTDOWN(Lattice);

    FCollada::Shutdown();

    POOL_TAG(Lattice)::Shutdown();
}
//----------------------------------------------------------------------------
void LatticeStartup::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Lattice);

    POOL_TAG(Lattice)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
