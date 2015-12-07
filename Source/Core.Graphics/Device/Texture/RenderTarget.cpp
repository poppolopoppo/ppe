#include "stdafx.h"

#include "RenderTarget.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RenderTarget::RenderTarget(size_t width, size_t height, const SurfaceFormat *format, bool sharable)
:   Texture2D(width, height, 1, format, BufferMode::None, BufferUsage::Default, sharable) {
    Assert(format->SupportRenderTarget());
}
//----------------------------------------------------------------------------
RenderTarget::~RenderTarget() {}
//----------------------------------------------------------------------------
void RenderTarget::Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(Width(), Height(), LevelCount()));
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D = device->CreateRenderTarget(this, optionalRawData);

    Assert(_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
void RenderTarget::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    PDeviceAPIDependantRenderTarget rt = checked_cast<Graphics::DeviceAPIDependantRenderTarget *>(_deviceAPIDependantTexture2D.get());
    _deviceAPIDependantTexture2D.reset();

    device->DestroyRenderTarget(this, rt);

    Assert(!rt);
}
//----------------------------------------------------------------------------
void RenderTarget::StealRenderTarget(Graphics::DeviceAPIDependantRenderTarget* rt) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(rt);
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D.reset(rt);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantRenderTarget::DeviceAPIDependantRenderTarget(IDeviceAPIEncapsulator *device, const RenderTarget *resource, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantTexture2D(device, resource, optionalData) {}
//----------------------------------------------------------------------------
DeviceAPIDependantRenderTarget::~DeviceAPIDependantRenderTarget() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
