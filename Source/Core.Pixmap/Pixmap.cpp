#include "stdafx.h"

#include "Pixmap.h"
#include "Pixmap_fwd.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace Pixmap {
POOL_TAG_DEF(Pixmap);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void PixmapStartup::Start() {
    POOL_TAG(Pixmap)::Start();
}
//----------------------------------------------------------------------------
void PixmapStartup::Shutdown() {
    POOL_TAG(Pixmap)::Shutdown();
}
//----------------------------------------------------------------------------
void PixmapStartup::Clear() {
    POOL_TAG(Pixmap)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
void PixmapStartup::ClearAll_UnusedMemory() {
    POOL_TAG(Pixmap)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
