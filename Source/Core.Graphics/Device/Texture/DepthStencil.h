#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Texture/Texture2D.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantDepthStencil);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DepthStencil);
class DepthStencil : public Texture2D {
public:
    DepthStencil(size_t width, size_t height, const SurfaceFormat *format, bool sharable);
    DepthStencil(size_t width, size_t height, const SurfaceFormat *format, bool sharable, DeviceAPIDependantDepthStencil *deviceAPIDependantDepthStencil);
    virtual ~DepthStencil();

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData);
    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

private:
    void Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData);
};
//----------------------------------------------------------------------------
template <typename T>
void DepthStencil::Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
inline void DepthStencil::Create(IDeviceAPIEncapsulator *device) {
    Create_(device, MemoryView<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantDepthStencil : public Graphics::DeviceAPIDependantTexture2D {
public:
    DeviceAPIDependantDepthStencil(IDeviceAPIEncapsulator *device, const DepthStencil *owner, const MemoryView<const u8>& optionalData);
    virtual ~DeviceAPIDependantDepthStencil();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
