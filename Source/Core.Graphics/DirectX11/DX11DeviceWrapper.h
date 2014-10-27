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

namespace DX11 {
class DeviceEncapsulator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceWrapper {
public:
    DeviceWrapper();
    ~DeviceWrapper();

    DeviceWrapper(const DeviceWrapper& ) = delete;
    DeviceWrapper& operator =(const DeviceWrapper& ) = delete;

    ::IDXGISwapChain *SwapChain() const { return _dx11SwapChain.Get(); }
    ::ID3D11Device *Device() const { return _dx11Device.Get(); }
    ::ID3D11DeviceContext *ImmediateContext() const { return _dx11ImmediateContext.Get(); }
    ::D3D_FEATURE_LEVEL FeatureLevel() const { return _dx11FeatureLevel; }

#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    ::ID3DUserDefinedAnnotation *UserDefinedAnnotation() const { return _dx11UserDefinedAnnotation.Get(); }
#endif

    const Graphics::PRenderTarget& BackBufferRenderTarget() const { return _backBufferRenderTarget; }
    const Graphics::PDepthStencil& BackBufferDepthStencil() const { return _backBufferDepthStencil; }

    void Create(DX11::DeviceEncapsulator *device, void *windowHandle, const PresentationParameters& presentationParameters);
    void Destroy(DX11::DeviceEncapsulator *device);

    void CheckDeviceErrors(const DX11::DeviceEncapsulator *encapsulator) const;

private:
    ComPtr<::ID3D11DeviceContext> _dx11ImmediateContext;
    ComPtr<::ID3D11Device> _dx11Device;
    ComPtr<::IDXGISwapChain> _dx11SwapChain;

    Graphics::PRenderTarget _backBufferRenderTarget;
    Graphics::PDepthStencil _backBufferDepthStencil;

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
        Core::Graphics::DX11::DX11ThrowIfFailed( \
            _ENCAPSULATOR, CONCAT(dx11Result_, __LINE__), _OPTIONAL_RESOURCE, \
            STRINGIZE(_CALL), \
            __FILE__, __LINE__, __FUNCTION__ \
            ); \
    }
//----------------------------------------------------------------------------
void DX11SetDeviceResourceName(::ID3D11DeviceChild *deviceChild, const char *name, size_t length);
void DX11SetDeviceResourceNameIFP(::ID3D11DeviceChild *deviceChild, const Graphics::DeviceResource *owner);
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
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
