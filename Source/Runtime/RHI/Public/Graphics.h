#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_GRAPHICS
#   define PPE_GRAPHICS_API DLL_EXPORT
#else
#   define PPE_GRAPHICS_API DLL_IMPORT
#endif

#include "Allocator/PoolAllocatorTag.h"

#define GRAPHICS_BOUNDARY (16)

namespace PPE {
namespace Graphics {
class FDeviceEncapsulator;
POOL_TAG_DECL(Graphics);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FGraphicsModule is the entry and exit point encapsulating every call to PPE::Graphics::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class PPE_GRAPHICS_API FGraphicsModule {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    // necessary for static graphics resources (fail ?)
    static void OnDeviceCreate(FDeviceEncapsulator *device);
    static void OnDeviceDestroy(FDeviceEncapsulator *device);

    FGraphicsModule()  { Start(); }
    ~FGraphicsModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
