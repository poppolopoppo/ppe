#include "stdafx.h"

#include "Pixmap.h"
#include "Pixmap_fwd.h"

#include "Image.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

PRAGMA_INITSEG_LIB

namespace Core {
namespace Pixmap {
POOL_TAG_DEF(Pixmap);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FPixmapModule::Start() {
    CORE_MODULE_START(Pixmap);

    POOL_TAG(Pixmap)::Start();
    FImage::Start();
}
//----------------------------------------------------------------------------
void FPixmapModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Pixmap);

    FImage::Shutdown();
    POOL_TAG(Pixmap)::Shutdown();
}
//----------------------------------------------------------------------------
void FPixmapModule::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Pixmap);

    POOL_TAG(Pixmap)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
