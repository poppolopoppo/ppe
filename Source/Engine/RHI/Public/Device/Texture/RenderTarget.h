#pragma once

#include "Graphics.h"

#include "Device/Texture/Texture2D.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantRenderTarget);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RenderTarget);
class FRenderTarget : public FTexture2D {
public:
    FRenderTarget(size_t width, size_t height, const FSurfaceFormat *format, bool sharable);
    virtual ~FRenderTarget();

    const Graphics::FDeviceAPIDependantRenderTarget *FDeviceAPIDependantRenderTarget() const {
        return checked_cast<const Graphics::FDeviceAPIDependantRenderTarget *>(FTexture2D::DeviceAPIDependantTexture2D().get());
    }

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData);
    void Create(IDeviceAPIEncapsulator *device) { Create_(device, TMemoryView<const u8>()); }

    void Destroy(IDeviceAPIEncapsulator *device);

    void StealRenderTarget(Graphics::FDeviceAPIDependantRenderTarget* rt);

private:
    void Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData);
};
//----------------------------------------------------------------------------
template <typename T>
void FRenderTarget::Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantRenderTarget : public Graphics::FDeviceAPIDependantTexture2D {
public:
    FDeviceAPIDependantRenderTarget(IDeviceAPIEncapsulator *device, const FRenderTarget *resource, const TMemoryView<const u8>& optionalData);
    virtual ~FDeviceAPIDependantRenderTarget();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FRenderTargetBinding {
    const FRenderTarget *RT;
    size_t Slot;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
