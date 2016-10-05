#include "stdafx.h"

#include "DX11DeviceWrapper.h"

#include "DX11DeviceAPIEncapsulator.h"

#include "Texture/DX11DepthStencil.h"
#include "Texture/DX11RenderTarget.h"
#include "Texture/DX11SurfaceFormat.h"

#include "Device/DeviceEncapsulator.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/PresentationParameters.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Core/Diagnostic/LastError.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Format.h"
#include "Core/IO/String.h"

#ifdef OS_WINDOWS
//#   include <DxErr.h>
//#   pragma comment(lib, "DxErr.lib")
#   include <DXGIDebug.h>
#else
#   error "no support"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool CreateDX11DeviceAndSwapChainIFP_(
    const FDX11DeviceAPIEncapsulator *encapsulator,
    TComPtr<::ID3D11DeviceContext>& dx11ImmediateContext,
    TComPtr<::ID3D11Device>& dx11Device,
    TComPtr<::IDXGISwapChain>& dx11SwapChain,
    ::D3D_FEATURE_LEVEL *dx11pFeatureLevel,
    void *windowHandle,
    const FPresentationParameters& presentationParameters ) {
    UINT refreshRateDenominator = 1;
    switch (presentationParameters.PresentationInterval())
    {
    case EPresentInterval::One:      refreshRateDenominator = 1; break;
    case EPresentInterval::Two:      refreshRateDenominator = 2; break;
    case EPresentInterval::Three:    refreshRateDenominator = 3; break;
    case EPresentInterval::Four:     refreshRateDenominator = 4; break;
    default:
        AssertNotImplemented();
    }

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef WITH_DIRECTX11_DEBUG_LAYER
    createDeviceFlags |= ::D3D11_CREATE_DEVICE_DEBUG;
#endif

    const ::D3D_DRIVER_TYPE driverTypes[] = {
#ifndef WITH_DIRECTX11_WARP_DRIVER
        ::D3D_DRIVER_TYPE_HARDWARE,
#endif
        ::D3D_DRIVER_TYPE_WARP,
        ::D3D_DRIVER_TYPE_REFERENCE,
    };

    const ::D3D_FEATURE_LEVEL featureLevels[] = {
        ::D3D_FEATURE_LEVEL_11_1,
        ::D3D_FEATURE_LEVEL_11_0,
        ::D3D_FEATURE_LEVEL_10_1,
        ::D3D_FEATURE_LEVEL_10_0,
    };

    ::DXGI_SWAP_CHAIN_DESC sd;
    ::SecureZeroMemory( &sd, sizeof(sd) );

    sd.BufferCount = presentationParameters.TripleBuffer() ? 3 : 2;

    sd.BufferDesc.Width = presentationParameters.BackBufferWidth();
    sd.BufferDesc.Height = presentationParameters.BackBufferHeight();
    sd.BufferDesc.Format = SurfaceFormatTypeToDXGIFormat(presentationParameters.BackBufferFormat());
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = refreshRateDenominator;

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = reinterpret_cast<HWND>(windowHandle);

    // Back buffer should never multi-sampled :
    AssertRelease(presentationParameters.MultiSampleCount() <= 1);
    sd.SampleDesc.Count = 1;//std::max(presentationParameters.MultiSampleCount(), u32(1));
    sd.SampleDesc.Quality = 0;//presentationParameters.MultiSampleCount() ? 1 : 0;

    sd.Windowed = presentationParameters.FullScreen() ? FALSE : TRUE;
    sd.SwapEffect = ::DXGI_SWAP_EFFECT_DISCARD;

    const HRESULT result = ::D3D11CreateDeviceAndSwapChain(
        NULL,
        driverTypes[0],
        NULL,
        createDeviceFlags,
        featureLevels, lengthof(featureLevels),
        D3D11_SDK_VERSION,
        &sd,
        dx11SwapChain.GetAddressOf(),
        dx11Device.GetAddressOf(),
        dx11pFeatureLevel,
        dx11ImmediateContext.GetAddressOf() );

    switch (result) {
    case DXGI_ERROR_SDK_COMPONENT_MISSING:
        CORE_THROW_IT(FDeviceEncapsulatorException("Windows SDK is not at the correct version", encapsulator->Device()));
    default:
        return SUCCEEDED(result);
    }
}
//----------------------------------------------------------------------------
#ifdef WITH_DIRECTX11_DEBUG_LAYER
static bool CreateDX11DebugLayerIFP_(
    ::ID3D11Debug **dx11pDebug,
    ::ID3D11InfoQueue **dx11pInfoQueue,
    const TComPtr<::ID3D11Device>& dx11Device ) {
    HRESULT result;

    result = dx11Device.Get()->QueryInterface(__uuidof(::ID3D11Debug), (LPVOID *)dx11pDebug);
    if (FAILED(result))
        return false;
    Assert(*dx11pDebug);

    result = (*dx11pDebug)->QueryInterface(__uuidof(::ID3D11InfoQueue), (LPVOID *)dx11pInfoQueue);
    if (FAILED(result))
        return false;
    Assert(*dx11pInfoQueue);

    ::D3D11_MESSAGE_SEVERITY severities[] = {
        ::D3D11_MESSAGE_SEVERITY_ERROR,
        ::D3D11_MESSAGE_SEVERITY_CORRUPTION,
        ::D3D11_MESSAGE_SEVERITY_WARNING
    };

    ::D3D11_INFO_QUEUE_FILTER_DESC allowList;
    ::SecureZeroMemory(&allowList, sizeof(allowList));
    allowList.NumSeverities = _countof(severities);
    allowList.pSeverityList = &severities[0];

    ::D3D11_INFO_QUEUE_FILTER_DESC denyList;
    ::D3D11_MESSAGE_ID hide [] = { ::D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS };
    ::SecureZeroMemory(&denyList, sizeof(denyList));
    denyList.NumIDs = _countof(hide);
    denyList.pIDList = hide;

    ::D3D11_INFO_QUEUE_FILTER filter;
    ::SecureZeroMemory(&filter, sizeof(filter));
    filter.AllowList = allowList;
    filter.DenyList = denyList;
    (*dx11pInfoQueue)->PushStorageFilter(&filter);

    if (::IsDebuggerPresent()) {
        (*dx11pInfoQueue)->SetBreakOnSeverity(::D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        (*dx11pInfoQueue)->SetBreakOnSeverity(::D3D11_MESSAGE_SEVERITY_ERROR, true);
    }

    return true;
}
#endif //!WITH_DIRECTX11_DEBUG_LAYER
//----------------------------------------------------------------------------
static FRenderTarget *CreateDX11BackBufferRenderTarget_(
    FDX11DeviceAPIEncapsulator *device,
    const TComPtr<::ID3D11Device>& dx11Device,
    const TComPtr<::IDXGISwapChain>& dx11SwapChain,
    const FPresentationParameters& presentationParameters ) {
    TComPtr<::ID3D11Texture2D> pBackBuffer;
    DX11_THROW_IF_FAILED(device->Device(), nullptr, (
        dx11SwapChain->GetBuffer(0, __uuidof(::ID3D11Texture2D), (LPVOID *)pBackBuffer.GetAddressOf())
        ));

    DX11SetDeviceResourceName(pBackBuffer, "BackBuffer");

    TComPtr<::ID3D11RenderTargetView> pBackBufferRenderTargetView;
    DX11_THROW_IF_FAILED(device->Device(), nullptr, (
        dx11Device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pBackBufferRenderTargetView.GetAddressOf())
        ));

    DX11SetDeviceResourceName(pBackBufferRenderTargetView, "BackBuffer");

    FRenderTarget *const backBufferRenderTarget = new FRenderTarget(
        presentationParameters.BackBufferWidth(),
        presentationParameters.BackBufferHeight(),
        FSurfaceFormat::FromType(presentationParameters.BackBufferFormat()),
        false );
    backBufferRenderTarget->SetResourceName("BackBufferRenderTarget");
    backBufferRenderTarget->Freeze();

    FDX11RenderTarget *dx11RenderTarget = new FDX11RenderTarget(device->Device(), backBufferRenderTarget, pBackBuffer.Get(), nullptr, pBackBufferRenderTargetView.Get());
    DX11SetDeviceResourceName(dx11RenderTarget->RenderTargetView(), "BackBufferRenderTarget");

    backBufferRenderTarget->StealRenderTarget(dx11RenderTarget);

    return backBufferRenderTarget;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11DeviceWrapper::FDX11DeviceWrapper() {}
//----------------------------------------------------------------------------
FDX11DeviceWrapper::~FDX11DeviceWrapper() {
    Assert(!_dx11SwapChain);
    Assert(!_dx11Device);
    Assert(!_dx11ImmediateContext);

    Assert(!_backBufferRenderTarget);
    Assert(!_backBufferDepthStencil);
}
//----------------------------------------------------------------------------
void FDX11DeviceWrapper::Create(FDX11DeviceAPIEncapsulator *device, void *windowHandle, const FPresentationParameters& presentationParameters) {
    Assert(!_dx11SwapChain);
    Assert(!_dx11Device);
    Assert(!_dx11ImmediateContext);

    Assert(!_backBufferRenderTarget);
    Assert(!_backBufferDepthStencil);

    Assert(device);
    Assert(windowHandle);

    if (false == CreateDX11DeviceAndSwapChainIFP_(  device,
                                                    _dx11ImmediateContext, _dx11Device, _dx11SwapChain,
                                                    &_dx11FeatureLevel,
                                                    windowHandle, presentationParameters) ) {
        CheckDeviceErrors(device);
    }

    // performance markers

#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    _dx11ImmediateContext->QueryInterface(IID_PPV_ARGS(_dx11UserDefinedAnnotation.GetAddressOf()));
#endif //!WITH_DIRECTX11_DEBUG_MARKERS

    // debug layer

#ifdef WITH_DIRECTX11_DEBUG_LAYER
    CreateDX11DebugLayerIFP_(&_dx11Debug, &_dx11InfoQueue, _dx11Device);
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

    _backBufferRenderTarget = CreateDX11BackBufferRenderTarget_( device, _dx11Device, _dx11SwapChain, presentationParameters);
    Assert(_backBufferRenderTarget);

    // create back buffer depth stencil IFN

    if (presentationParameters.DepthStencilFormat() != ESurfaceFormatType::UNKNOWN) {
        _backBufferDepthStencil = new FDepthStencil(
            _backBufferRenderTarget->Width(), _backBufferRenderTarget->Height(),
            FSurfaceFormat::FromType(presentationParameters.DepthStencilFormat()),
            false );
        _backBufferDepthStencil->SetResourceName("BackBufferDepthStencil");
        _backBufferDepthStencil->Freeze();
        _backBufferDepthStencil->Create(device->Device());
    }

    // final checks

    Assert(_dx11SwapChain);
    Assert(_dx11Device);
    Assert(_dx11ImmediateContext);

    Assert(_backBufferRenderTarget);
    Assert(ESurfaceFormatType::UNKNOWN == presentationParameters.BackBufferFormat() || _backBufferDepthStencil);

    CheckDeviceErrors(device);
}
//----------------------------------------------------------------------------
void FDX11DeviceWrapper::Destroy(FDX11DeviceAPIEncapsulator *device) {
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
        _dx11Debug->ReportLiveDeviceObjects(::D3D11_RLDO_DETAIL);
    }
#endif

    ReleaseComRef(_dx11Device);
}
//----------------------------------------------------------------------------
void FDX11DeviceWrapper::CheckDeviceErrors(const FDX11DeviceAPIEncapsulator *encapsulator) const {
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
        case ::D3D11_MESSAGE_SEVERITY_MESSAGE:
        case ::D3D11_MESSAGE_SEVERITY_INFO:
            LOG(Info, L"[D3D11] {0}", message->pDescription);
            break;

        case ::D3D11_MESSAGE_SEVERITY_WARNING:
            LOG(Warning, L"[D3D11] {0}", formatedMessage);
            break;

        case ::D3D11_MESSAGE_SEVERITY_ERROR:
        case ::D3D11_MESSAGE_SEVERITY_CORRUPTION:
            LOG(Error, L"[D3D11] {0}", formatedMessage);
            CORE_THROW_IT(FDeviceEncapsulatorException(formatedMessage, encapsulator->Device()));
            break;
        }
    }

    _dx11InfoQueue->ClearStoredMessages();

#else
    UNUSED(encapsulator);

#endif //!WITH_DIRECTX11_DEBUG_LAYER
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DX11ThrowIfFailed(
    const IDeviceAPIEncapsulator *encapsulator,
    HRESULT result,
    const FDeviceResource *optionalResource,
    const char *call,
    const char *file, const size_t line,
    const char *func) {
    if ( SUCCEEDED(result) )
        return;

    Assert(encapsulator);
    Assert(file);
    Assert(func);

    const FString dx11Message = GetLastErrorToString((long)result);

    char formatedMessage[1024];
    Format(formatedMessage,
        "{0}\n"
        "   Call  = {1}\n"
        "   At    = {2}({3})\n"
        "   In    = {4}()",
        dx11Message,
        call,
        file, line,
        func
        );

    LOG(Error, L"[D3D11] {0}", formatedMessage);
    CORE_THROW_IT(FDeviceEncapsulatorException(formatedMessage, encapsulator, optionalResource));
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
#else
    UNUSED(deviceChild);
    UNUSED(name);
    UNUSED(length);
#endif
}
//----------------------------------------------------------------------------
void DX11SetDeviceResourceNameIFP(::ID3D11DeviceChild *deviceChild, const FDeviceResource *owner) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(owner);
    if (!deviceChild)
        return;

    const FStringView resourceName = owner->ResourceName();
    if (resourceName.empty())
        return;

    const UINT length = checked_cast<UINT>(resourceName.size());
    deviceChild->SetPrivateData(WKPDID_D3DDebugObjectName, length, resourceName.data());
#else
    UNUSED(deviceChild);
    UNUSED(owner);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
