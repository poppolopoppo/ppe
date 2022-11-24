// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Pixmap.h"
#include "Pixmap_fwd.h"

#include "Image.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "Diagnostic/Logger.h"
#include "IO/StringView.h"

PRAGMA_INITSEG_LIB

namespace PPE {
namespace Pixmap {
POOL_TAG_DEF(Pixmap);
LOG_CATEGORY(PPE_PIXMAP_API, Pixmap)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FPixmapModule::Start() {
    PPE_MODULE_START(Pixmap);

    POOL_TAG(Pixmap)::Start();
    FImage::Start();
}
//----------------------------------------------------------------------------
void FPixmapModule::Shutdown() {
    PPE_MODULE_SHUTDOWN(Pixmap);

    FImage::Shutdown();
    POOL_TAG(Pixmap)::Shutdown();
}
//----------------------------------------------------------------------------
void FPixmapModule::ClearAll_UnusedMemory() {
    PPE_MODULE_CLEARALL(Pixmap);

    POOL_TAG(Pixmap)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace PPE
