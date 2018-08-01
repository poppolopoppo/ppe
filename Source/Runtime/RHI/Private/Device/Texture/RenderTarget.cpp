#include "stdafx.h"

#include "RenderTarget.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderTarget::FRenderTarget(size_t width, size_t height, const FSurfaceFormat *format, bool sharable)
:   FTexture2D(width, height, 1, format, EBufferMode::None, EBufferUsage::Default, sharable) {
    Assert(format->SupportRenderTarget());
}
//----------------------------------------------------------------------------
FRenderTarget::~FRenderTarget() {}
//----------------------------------------------------------------------------
void FRenderTarget::Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(Width(), Height(), LevelCount()));
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D = device->CreateRenderTarget(this, optionalRawData);

    Assert(_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
void FRenderTarget::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    PDeviceAPIDependantRenderTarget rt = checked_cast<Graphics::FDeviceAPIDependantRenderTarget *>(_deviceAPIDependantTexture2D.get());
    _deviceAPIDependantTexture2D.reset();

    device->DestroyRenderTarget(this, rt);

    Assert(!rt);
}
//----------------------------------------------------------------------------
void FRenderTarget::StealRenderTarget(Graphics::FDeviceAPIDependantRenderTarget* rt) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(rt);
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D.reset(rt);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantRenderTarget::FDeviceAPIDependantRenderTarget(IDeviceAPIEncapsulator *device, const FRenderTarget *resource, const TMemoryView<const u8>& optionalData)
:   FDeviceAPIDependantTexture2D(device, resource, optionalData) {}
//----------------------------------------------------------------------------
FDeviceAPIDependantRenderTarget::~FDeviceAPIDependantRenderTarget() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
