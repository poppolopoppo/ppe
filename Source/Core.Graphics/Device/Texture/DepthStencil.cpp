#include "stdafx.h"

#include "DepthStencil.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDepthStencil::FDepthStencil(size_t width, size_t height, const FSurfaceFormat *format, bool sharable)
:   FTexture2D(width, height, 1, format, EBufferMode::None, EBufferUsage::Default, sharable) {
    Assert(format->SupportDepthStencil());
}
//----------------------------------------------------------------------------
FDepthStencil::FDepthStencil(size_t width, size_t height, const FSurfaceFormat *format, bool sharable, FDeviceAPIDependantDepthStencil *deviceAPIDependantDepthStencil)
:   FTexture2D(width, height, 1, format, EBufferMode::None, EBufferUsage::Default, sharable, deviceAPIDependantDepthStencil) {}
//----------------------------------------------------------------------------
FDepthStencil::~FDepthStencil() {}
//----------------------------------------------------------------------------
void FDepthStencil::Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(Width(), Height(), LevelCount()));
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D = device->CreateDepthStencil(this, optionalRawData);

    Assert(_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
void FDepthStencil::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    PDeviceAPIDependantDepthStencil ds = checked_cast<FDeviceAPIDependantDepthStencil *>(_deviceAPIDependantTexture2D.get());
    _deviceAPIDependantTexture2D.reset();

    device->DestroyDepthStencil(this, ds);

    Assert(!ds);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantDepthStencil::FDeviceAPIDependantDepthStencil(IDeviceAPIEncapsulator *device, const FDepthStencil *resource, const TMemoryView<const u8>& optionalData)
:   FDeviceAPIDependantTexture2D(device, resource, optionalData) {}
//----------------------------------------------------------------------------
FDeviceAPIDependantDepthStencil::~FDeviceAPIDependantDepthStencil() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
