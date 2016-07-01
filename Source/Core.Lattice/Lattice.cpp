#include "stdafx.h"

#include "Lattice.h"
#include "Lattice_fwd.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
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
    POOL_TAG(Lattice)::Start();
}
//----------------------------------------------------------------------------
void LatticeStartup::Shutdown() {
    POOL_TAG(Lattice)::Shutdown();
}
//----------------------------------------------------------------------------
void LatticeStartup::Clear() {
    POOL_TAG(Lattice)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
void LatticeStartup::ClearAll_UnusedMemory() {
    POOL_TAG(Lattice)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
