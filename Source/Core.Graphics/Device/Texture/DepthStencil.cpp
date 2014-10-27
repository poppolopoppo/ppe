#include "stdafx.h"

#include "DepthStencil.h"

#include "Device/DeviceAPIEncapsulator.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DepthStencil::DepthStencil(
    size_t width,
    size_t height,
    const SurfaceFormat *format
    )
:   Texture2D(width, height, 1, format, BufferMode::None, BufferUsage::Default) {
    Assert(format->SupportDepthStencil());
}
//----------------------------------------------------------------------------
DepthStencil::DepthStencil(size_t width, size_t height, const SurfaceFormat *format, DeviceAPIDependantDepthStencil *deviceAPIDependantDepthStencil)
:   Texture2D(width, height, 1, format, BufferMode::None, BufferUsage::Default, deviceAPIDependantDepthStencil) {}
//----------------------------------------------------------------------------
DepthStencil::~DepthStencil() {}
//----------------------------------------------------------------------------
void DepthStencil::Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(Width(), Height(), LevelCount()));
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D = device->CreateDepthStencil(this, optionalRawData);

    Assert(_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
void DepthStencil::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    PDeviceAPIDependantDepthStencil ds = checked_cast<DeviceAPIDependantDepthStencil *>(_deviceAPIDependantTexture2D.get());
    _deviceAPIDependantTexture2D.reset();

    device->DestroyDepthStencil(this, ds);

    Assert(!ds);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencil::DeviceAPIDependantDepthStencil(IDeviceAPIEncapsulator *device, DepthStencil *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantTexture2D(device, owner, optionalData) {}
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencil::~DeviceAPIDependantDepthStencil() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
