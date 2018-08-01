#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_PIXMAP
#   define PPE_PIXMAP_API DLL_EXPORT
#else
#   define PPE_PIXMAP_API DLL_IMPORT
#endif

namespace PPE {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_PIXMAP_API FPixmapModule {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FPixmapModule() { Start(); }
    ~FPixmapModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace PPE
