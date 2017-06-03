#pragma once

#include "Core.Pixmap/Pixmap.h"

#ifdef EXPORT_CORE_PIXMAP
#   define CORE_PIXMAP_API DLL_EXPORT
#else
#   define CORE_PIXMAP_API DLL_IMPORT
#endif

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_PIXMAP_API FPixmapModule {
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
} //!namespace Core
