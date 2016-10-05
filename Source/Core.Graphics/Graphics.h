#pragma once

#include "Core/Core.h"

#ifdef EXPORT_CORE_GRAPHICS
#   define CORE_GRAPHICS_API DLL_EXPORT
#else
#   define CORE_GRAPHICS_API DLL_IMPORT
#endif

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
namespace Graphics {
class FDeviceEncapsulator;
POOL_TAG_DECL(Graphics);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// GraphicsStartup is the entry and exit point encapsulating every call to Core::Graphics::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class CORE_GRAPHICS_API GraphicsStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    // necessary for static graphics resources (fail ?)
    static void OnDeviceCreate(FDeviceEncapsulator *device);
    static void OnDeviceDestroy(FDeviceEncapsulator *device);

    GraphicsStartup()  { Start(); }
    ~GraphicsStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
