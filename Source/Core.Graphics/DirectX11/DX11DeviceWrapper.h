#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#ifdef WITH_DIRECTX11_DEBUG_MARKERS
#   include <D3D11_1.h> // ID3DUserDefinedAnnotation interface
#endif

#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class DeviceResource;
class IDeviceAPIEncapsulator;
class PresentationParameters;

FWD_REFPTR(RenderTarget);
FWD_REFPTR(DepthStencil);

class DX11DeviceAPIEncapsulator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11DeviceWrapper {
public:
    DX11DeviceWrapper();
    ~DX11DeviceWrapper();

    DX11DeviceWrapper(const DX11DeviceWrapper& ) = delete;
    DX11DeviceWrapper& operator =(const DX11DeviceWrapper& ) = delete;

    ::IDXGISwapChain *SwapChain() const { return _dx11SwapChain.Get(); }
    ::ID3D11Device *Device() const { return _dx11Device.Get(); }
    ::ID3D11DeviceContext *ImmediateContext() const { return _dx11ImmediateContext.Get(); }
    ::D3D_FEATURE_LEVEL FeatureLevel() const { return _dx11FeatureLevel; }

#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    ::ID3DUserDefinedAnnotation *UserDefinedAnnotation() const { return _dx11UserDefinedAnnotation.Get(); }
#endif

    const PRenderTarget& BackBufferRenderTarget() const { return _backBufferRenderTarget; }
    const PDepthStencil& BackBufferDepthStencil() const { return _backBufferDepthStencil; }

    void Create(DX11DeviceAPIEncapsulator *device, void *windowHandle, const PresentationParameters& presentationParameters);
    void Destroy(DX11DeviceAPIEncapsulator *device);

    void CheckDeviceErrors(const DX11DeviceAPIEncapsulator *device) const;

private:
    ComPtr<::ID3D11DeviceContext> _dx11ImmediateContext;
    ComPtr<::ID3D11Device> _dx11Device;
    ComPtr<::IDXGISwapChain> _dx11SwapChain;

    PRenderTarget _backBufferRenderTarget;
    PDepthStencil _backBufferDepthStencil;

    ::D3D_FEATURE_LEVEL _dx11FeatureLevel;

#ifdef WITH_DIRECTX11_DEBUG_LAYER
    ::ID3D11Debug *_dx11Debug;
    ::ID3D11InfoQueue *_dx11InfoQueue;
#endif

#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    ComPtr<::ID3DUserDefinedAnnotation> _dx11UserDefinedAnnotation;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DX11ThrowIfFailed(
    const IDeviceAPIEncapsulator *encapsulator,
    HRESULT result,
    const DeviceResource *optionalResource,
    const char *call,
    const char *file, const size_t line,
    const char *func);
//----------------------------------------------------------------------------
#define DX11_THROW_IF_FAILED(_ENCAPSULATOR, _OPTIONAL_RESOURCE, _CALL) { \
    const HRESULT CONCAT(dx11Result_, __LINE__) = _CALL; \
    if (FAILED(CONCAT(dx11Result_, __LINE__))) \
        Core::Graphics::DX11ThrowIfFailed( \
            _ENCAPSULATOR, CONCAT(dx11Result_, __LINE__), _OPTIONAL_RESOURCE, \
            STRINGIZE(_CALL), \
            __FILE__, __LINE__, __FUNCTION__ \
            ); \
    }
//----------------------------------------------------------------------------
void DX11SetDeviceResourceName(::ID3D11DeviceChild *deviceChild, const char *name, size_t length);
void DX11SetDeviceResourceNameIFP(::ID3D11DeviceChild *deviceChild, const DeviceResource *owner);
//----------------------------------------------------------------------------
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
template <size_t _Dim>
void DX11SetDeviceResourceName(::ID3D11DeviceChild *deviceChild, const char (&staticArray)[_Dim]) {
    DX11SetDeviceResourceName(deviceChild, staticArray, _Dim);
}
#else
//  Prevent useless static string export in the binary
template <size_t _Dim>
void DX11SetDeviceResourceName(::ID3D11DeviceChild *, const char (&)[_Dim]) {}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
