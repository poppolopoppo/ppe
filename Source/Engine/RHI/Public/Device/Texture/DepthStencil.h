#pragma once

#include "Graphics.h"

#include "Device/Texture/Texture2D.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantDepthStencil);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DepthStencil);
class FDepthStencil : public FTexture2D {
public:
    FDepthStencil(size_t width, size_t height, const FSurfaceFormat *format, bool sharable);
    FDepthStencil(size_t width, size_t height, const FSurfaceFormat *format, bool sharable, FDeviceAPIDependantDepthStencil *deviceAPIDependantDepthStencil);
    virtual ~FDepthStencil();

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData);
    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

private:
    void Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData);
};
//----------------------------------------------------------------------------
template <typename T>
void FDepthStencil::Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
inline void FDepthStencil::Create(IDeviceAPIEncapsulator *device) {
    Create_(device, TMemoryView<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantDepthStencil : public Graphics::FDeviceAPIDependantTexture2D {
public:
    FDeviceAPIDependantDepthStencil(IDeviceAPIEncapsulator *device, const FDepthStencil *owner, const TMemoryView<const u8>& optionalData);
    virtual ~FDeviceAPIDependantDepthStencil();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
