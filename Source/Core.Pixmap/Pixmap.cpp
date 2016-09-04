#include "stdafx.h"

#include "Pixmap.h"
#include "Pixmap_fwd.h"

#include "Image.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#ifdef CPP_VISUALSTUDIO
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
    CORE_MODULE_START(Pixmap);

    POOL_TAG(Pixmap)::Start();
    Image::Start();
}
//----------------------------------------------------------------------------
void PixmapStartup::Shutdown() {
    CORE_MODULE_SHUTDOWN(Pixmap);

    Image::Shutdown();
    POOL_TAG(Pixmap)::Shutdown();
}
//----------------------------------------------------------------------------
void PixmapStartup::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Pixmap);

    POOL_TAG(Pixmap)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
