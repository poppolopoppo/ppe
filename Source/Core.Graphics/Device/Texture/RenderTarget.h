#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Texture/Texture2D.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantRenderTarget);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RenderTarget);
class RenderTarget : public Texture2D {
public:
    RenderTarget(size_t width, size_t height, const SurfaceFormat *format, bool sharable);
    virtual ~RenderTarget();

    const Graphics::DeviceAPIDependantRenderTarget *DeviceAPIDependantRenderTarget() const {
        return checked_cast<const Graphics::DeviceAPIDependantRenderTarget *>(Texture2D::DeviceAPIDependantTexture2D().get());
    }

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData);
    void Create(IDeviceAPIEncapsulator *device) { Create_(device, MemoryView<const u8>()); }

    void Destroy(IDeviceAPIEncapsulator *device);

    void StealRenderTarget(Graphics::DeviceAPIDependantRenderTarget* rt);

private:
    void Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData);
};
//----------------------------------------------------------------------------
template <typename T>
void RenderTarget::Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantRenderTarget : public Graphics::DeviceAPIDependantTexture2D {
public:
    DeviceAPIDependantRenderTarget(IDeviceAPIEncapsulator *device, const RenderTarget *resource, const MemoryView<const u8>& optionalData);
    virtual ~DeviceAPIDependantRenderTarget();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct RenderTargetBinding {
    const RenderTarget *RT;
    size_t Slot;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
