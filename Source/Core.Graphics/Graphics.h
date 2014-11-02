#pragma once

#include "Core/Core.h"

namespace Core {
namespace Graphics {
class DeviceEncapsulator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// CoreStartup is the entry and exit point encapsulating every call to Core::Graphics::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class GraphicsStartup {
public:
    static void Start();
    static void Shutdown();

    // necessary for static graphics resources (fail ?)
    static void OnDeviceCreate(DeviceEncapsulator *device);
    static void OnDeviceDestroy(DeviceEncapsulator *device);

    GraphicsStartup()  { Start(); }
    ~GraphicsStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
