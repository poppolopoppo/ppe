#include "stdafx.h"

#include "DX11DeviceWrapper.h"

#include "DX11DeviceEncapsulator.h"
#include "Texture/DX11DepthStencil.h"
#include "Texture/DX11RenderTarget.h"
#include "Texture/DX11SurfaceFormat.h"

#include "Device/DeviceEncapsulator.h"
#include "Device/PresentationParameters.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Core/Diagnostic/LastError.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Format.h"
#include "Core/IO/String.h"

#ifdef OS_WINDOWS
#   include <DxErr.h>
#   include <DXGIDebug.h>

#   pragma comment(lib, "DxErr.lib")
#else
#   error "no support"
#endif

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceWrapper::DeviceWrapper() {}
//----------------------------------------------------------------------------
DeviceWrapper::~DeviceWrapper() {
    Assert(!_dx11SwapChain);
    Assert(!_dx11Device);
    Assert(!_dx11ImmediateContext);

    Assert(!_backBufferRenderTarget);
    Assert(!_backBufferDepthStencil);
}
//----------------------------------------------------------------------------
void DeviceWrapper::Create(DX11::DeviceEncapsulator *device, void *windowHandle, const PresentationParameters& presentationParameters) {
    Assert(!_dx11SwapChain);
    Assert(!_dx11Device);
    Assert(!_dx11ImmediateContext);

    Assert(!_backBufferRenderTarget);
    Assert(!_backBufferDepthStencil);

    Assert(device);
    Assert(windowHandle);

    UINT refreshRateDenominator = 1;
    switch (presentationParameters.PresentationInterval())
    {
    case PresentInterval::One:      refreshRateDenominator = 1; break;
    case PresentInterval::Two:      refreshRateDenominator = 2; break;
    case PresentInterval::Three:    refreshRateDenominator = 3; break;
    case PresentInterval::Four:     refreshRateDenominator = 4; break;
    default:
        AssertNotImplemented();
    }

    UINT createDeviceFlags = 0;
#ifdef WITH_DIRECTX11_DEBUG_LAYER
    createDeviceFlags |= ::D3D11_CREATE_DEVICE_DEBUG;
#endif

    const ::D3D_DRIVER_TYPE driverTypes[] = {
        ::D3D_DRIVER_TYPE_HARDWARE,
        ::D3D_DRIVER_TYPE_WARP,
        ::D3D_DRIVER_TYPE_REFERENCE,
    };

    const ::D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    ::DXGI_SWAP_CHAIN_DESC sd;
    ::SecureZeroMemory( &sd, sizeof(sd) );

    sd.BufferCount = presentationParameters.TripleBuffer() ? 3 : 2;

    sd.BufferDesc.Width = presentationParameters.BackBufferWidth();
    sd.BufferDesc.Height = presentationParameters.BackBufferHeight();
    sd.BufferDesc.Format = SurfaceFormatTypeToDXGIFormat(presentationParameters.BackBufferFormat()->Type());
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = refreshRateDenominator;

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = reinterpret_cast<HWND>(windowHandle);

    sd.SampleDesc.Count = std::max(presentationParameters.MultiSampleCount(), u32(1));
    sd.SampleDesc.Quality = presentationParameters.MultiSampleCount() ? 1 : 0;

    sd.Windowed = presentationParameters.FullScreen() ? FALSE : TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if( FAILED(::D3D11CreateDeviceAndSwapChain(
            NULL,
            driverTypes[0],
            NULL,
            createDeviceFlags,
            featureLevels, lengthof(featureLevels),
            D3D11_SDK_VERSION,
            &sd,
            _dx11SwapChain.GetAddressOf(),
            _dx11Device.GetAddressOf(),
            &_dx11FeatureLevel,
            _dx11ImmediateContext.GetAddressOf())) )
    {
        CheckDeviceErrors(device);
    }

    // performance markers

#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    _dx11ImmediateContext->QueryInterface(IID_PPV_ARGS(_dx11UserDefinedAnnotation.GetAddressOf()));
#endif //!WITH_DIRECTX11_DEBUG_MARKERS

    // debug layer

#ifdef WITH_DIRECTX11_DEBUG_LAYER
    if( SUCCEEDED( _dx11Device.Get()->QueryInterface(__uuidof(::ID3D11Debug), (LPVOID *)&_dx11Debug)) ) {
        if( SUCCEEDED( _dx11Debug->QueryInterface(__uuidof(::ID3D11InfoQueue), (LPVOID *)&_dx11InfoQueue)) ) {
            ::D3D11_INFO_QUEUE_FILTER filter = { 0 };

            ::D3D11_INFO_QUEUE_FILTER_DESC allowList;
            ::D3D11_MESSAGE_SEVERITY severities[] = {D3D11_MESSAGE_SEVERITY_ERROR, D3D11_MESSAGE_SEVERITY_CORRUPTION, D3D11_MESSAGE_SEVERITY_WARNING };
            ::SecureZeroMemory(&allowList, sizeof(allowList));
            allowList.NumSeverities = _countof(severities);
            allowList.pSeverityList = &severities[0];

            ::D3D11_INFO_QUEUE_FILTER_DESC denyList;
            ::D3D11_MESSAGE_ID hide [] = { D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS };
            ::SecureZeroMemory(&denyList, sizeof(denyList));
            denyList.NumIDs = _countof(hide);
            denyList.pIDList = hide;

            filter.AllowList = allowList;
            filter.DenyList = denyList;

            _dx11InfoQueue->PushStorageFilter(&filter);

#   ifdef _DEBUG
            _dx11InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            //_dx11InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#   endif
        }
    }
#endif //!WITH_DIRECTX11_DEBUG_LAYER

    // set viewport

    ::D3D11_VIEWPORT dx11Viewport;
    dx11Viewport.TopLeftX = presentationParameters.Viewport().Left();
    dx11Viewport.TopLeftY = presentationParameters.Viewport().Top();
    dx11Viewport.Width = presentationParameters.Viewport().Width();
    dx11Viewport.Height = presentationParameters.Viewport().Height();
    dx11Viewport.MinDepth = presentationParameters.Viewport().Near();
    dx11Viewport.MaxDepth = presentationParameters.Viewport().Far();

    _dx11ImmediateContext->RSSetViewports(1, &dx11Viewport);

    // create back buffer render target

    ComPtr<::ID3D11Texture2D> pBackBuffer;
    DX11_THROW_IF_FAILED(device->Device(), nullptr, (
        _dx11SwapChain->GetBuffer(0, __uuidof(::ID3D11Texture2D), (LPVOID *)pBackBuffer.GetAddressOf())
        ));

    DX11SetDeviceResourceName(pBackBuffer, "BackBuffer");

    ComPtr<::ID3D11RenderTargetView> pBackBufferRenderTargetView;
    DX11_THROW_IF_FAILED(device->Device(), nullptr, (
        _dx11Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pBackBufferRenderTargetView.GetAddressOf())
        ));

    DX11SetDeviceResourceName(pBackBufferRenderTargetView, "BackBuffer");

    Graphics::RenderTarget *const backBufferStorage = reinterpret_cast<Graphics::RenderTarget *>(operator new(sizeof(Graphics::RenderTarget)));
    DX11::RenderTarget *const dx11RenderTarget = new DX11::RenderTarget(device->Device(), backBufferStorage, pBackBuffer.Get(), nullptr, pBackBufferRenderTargetView.Get());

    DX11SetDeviceResourceName(dx11RenderTarget->RenderTargetView(), "BackBufferRenderTarget");

    _backBufferRenderTarget = new ((void *)backBufferStorage) Graphics::RenderTarget(
        presentationParameters.BackBufferWidth(),
        presentationParameters.BackBufferHeight(),
        presentationParameters.BackBufferFormat(),
        dx11RenderTarget);
    _backBufferRenderTarget->SetResourceName("BackBufferRenderTarget");
    _backBufferRenderTarget->Freeze();

    // create back buffer depth stencil IFN

    if (presentationParameters.DepthStencilFormat()) {
        _backBufferDepthStencil = new Graphics::DepthStencil(_backBufferRenderTarget->Width(), _backBufferRenderTarget->Height(), presentationParameters.DepthStencilFormat());
        _backBufferDepthStencil->SetResourceName("BackBufferDepthStencil");
        _backBufferDepthStencil->Freeze();
        _backBufferDepthStencil->Create(device->Device());
    }

    Assert(_dx11SwapChain);
    Assert(_dx11Device);
    Assert(_dx11ImmediateContext);

    Assert(_backBufferRenderTarget);
    Assert(!presentationParameters.BackBufferFormat() || _backBufferDepthStencil);

    CheckDeviceErrors(device);
}
//----------------------------------------------------------------------------
void DeviceWrapper::Destroy(DX11::DeviceEncapsulator *device) {
    Assert(_dx11SwapChain);
    Assert(_dx11Device);
    Assert(_dx11ImmediateContext);

#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    _dx11UserDefinedAnnotation.Reset();
#endif

    Assert(_backBufferRenderTarget);
    _backBufferRenderTarget->Destroy(device->Device());
    RemoveRef_AssertReachZero(_backBufferRenderTarget);

    if (_backBufferDepthStencil) {
        _backBufferDepthStencil->Destroy(device->Device());
        RemoveRef_AssertReachZero(_backBufferDepthStencil);
    }

    if (_dx11ImmediateContext) {
        _dx11ImmediateContext->ClearState();
        _dx11ImmediateContext->Flush();
        ReleaseComRef(_dx11ImmediateContext);
    }

    if (_dx11SwapChain) {
        _dx11SwapChain->SetFullscreenState(FALSE, NULL);
        ReleaseComRef(_dx11SwapChain);
    }

#ifdef WITH_DIRECTX11_DEBUG_LAYER
    if (_dx11InfoQueue)
        _dx11InfoQueue->Release();

    if (_dx11Debug) {
        _dx11Debug->Release();
        _dx11Debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
#endif

    ReleaseComRef(_dx11Device);
}
//----------------------------------------------------------------------------
void DeviceWrapper::CheckDeviceErrors(const DX11::DeviceEncapsulator *encapsulator) const {
#ifdef WITH_DIRECTX11_DEBUG_LAYER

    if(!_dx11InfoQueue)
        return;

    const u64 messageCount = _dx11InfoQueue->GetNumStoredMessages();

    u8 messageData[1024];
    D3D11_MESSAGE *const message = reinterpret_cast<D3D11_MESSAGE*>(&messageData[0]);

    char formatedMessage[1024];

    SIZE_T messageLength = 0;
    for(u64 i = 0; i < messageCount ; i++)
    {
        _dx11InfoQueue->GetMessage(i, NULL, &messageLength);
        Assert(messageLength < lengthof(messageData));

        _dx11InfoQueue->GetMessage(i, message, &messageLength);

        Format(formatedMessage,
            "{0}\n"
            "\tId = {1}\n"
            "\tCategory = {2}\n"
            "\tSeverity = {3}\n"
            "\tGetDeviceRemovedReason() = {4}",
            message->pDescription,
            message->ID,
            message->Category,
            message->Severity,
            _dx11Device->GetDeviceRemovedReason() );

        switch(message->Severity)
        {
        case D3D11_MESSAGE_SEVERITY_INFO:
            LOG(Information, L"[D3D11] {0}", message->pDescription);
            break;

        case D3D11_MESSAGE_SEVERITY_WARNING:
            LOG(Warning, L"[D3D11] {0}", formatedMessage);
            break;

        case D3D11_MESSAGE_SEVERITY_ERROR:
        case D3D11_MESSAGE_SEVERITY_CORRUPTION:
            LOG(Error, L"[D3D11] {0}", formatedMessage);
            throw DeviceEncapsulatorException(formatedMessage, encapsulator->Device());
            break;
        }
    }

    _dx11InfoQueue->ClearStoredMessages();

#endif //!WITH_DIRECTX11_DEBUG_LAYER
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DX11ThrowIfFailed(
    const IDeviceAPIEncapsulator *encapsulator,
    HRESULT result,
    const DeviceResource *optionalResource,
    const char *call,
    const char *file, const size_t line,
    const char *func) {
    if ( SUCCEEDED(result) )
        return;

    Assert(encapsulator);
    Assert(file);
    Assert(func);

    const char *dx11Message = DXGetErrorStringA(result);
    const char *dx11Description = DXGetErrorDescriptionA(result);

    char formatedMessage[1024];
    Format(formatedMessage,
        "{0}\n"
        "{1}\n"
        "   Call  = {2}\n"
        "   At    = {3}({4})\n"
        "   In    = {5}()",
        dx11Message, dx11Description,
        call,
        file, line,
        func
        );

    LOG(Error, L"[D3D11] {0}", formatedMessage);
    throw DeviceEncapsulatorException(formatedMessage, encapsulator, optionalResource);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DX11SetDeviceResourceName(::ID3D11DeviceChild *deviceChild, const char *name, size_t length) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(deviceChild);
    Assert(name);
    Assert(length);

    deviceChild->SetPrivateData(WKPDID_D3DDebugObjectName, checked_cast<UINT>(length), name);
#endif
}
//----------------------------------------------------------------------------
void DX11SetDeviceResourceNameIFP(::ID3D11DeviceChild *deviceChild, const Graphics::DeviceResource *owner) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(owner);
    if (!deviceChild)
        return;

    const char *resourceName = owner->ResourceName();
    if (!resourceName)
        return;

    const UINT length = checked_cast<UINT>(Length(resourceName));
    deviceChild->SetPrivateData(WKPDID_D3DDebugObjectName, length, resourceName);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
