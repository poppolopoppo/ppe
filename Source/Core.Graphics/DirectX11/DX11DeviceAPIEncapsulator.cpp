#include "stdafx.h"

#include "DX11DeviceAPIEncapsulator.h"

#include "DX11DeviceWrapper.h"
#include "DX11ResourceBuffer.h"

#include "Geometry/DX11PrimitiveType.h"
#include "Geometry/DX11VertexDeclaration.h"

#include "Shader/DX11ConstantWriter.h"
#include "Shader/DX11ShaderProgram.h"

#include "State/DX11BlendState.h"
#include "State/DX11DepthStencilState.h"
#include "State/DX11RasterizerState.h"
#include "State/DX11SamplerState.h"

#include "Shader/DX11ShaderEffect.h"
#include "Shader/DX11ShaderProgram.h"

#include "Texture/DX11DepthStencil.h"
#include "Texture/DX11RenderTarget.h"
#include "Texture/DX11SurfaceFormat.h"
#include "Texture/DX11Texture2D.h"
#include "Texture/DX11TextureCube.h"

#include "Device/DeviceEncapsulatorException.h"
#include "Device/Geometry/IndexBuffer.h"
#include "Device/Geometry/VertexBuffer.h"
#include "Device/PresentationParameters.h"

#include "Core/Color/Color.h"
#include "Core/Maths/ScalarRectangle.h"
#include "Core/Memory/UniqueView.h"

#ifdef WITH_DIRECTX11_DEBUG_LAYER
#   define CHECK_DIRECTX11_ERROR() _wrapper.CheckDeviceErrors(this)
#else
#   define CHECK_DIRECTX11_ERROR()
#endif

#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
#   pragma comment( lib, "dxguid.lib") // WKPDID_D3DDebugObjectName link error
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FDX11DeviceWrapper *DX11GetDeviceWrapper(const IDeviceAPIEncapsulator *device) {
    return &checked_cast<const FDX11DeviceAPIEncapsulator *>(device->APIEncapsulator())->Wrapper();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11DeviceAPIEncapsulator::FDX11DeviceAPIEncapsulator(FDeviceEncapsulator *owner, void *windowHandle, const FPresentationParameters& pp)
:   FAbstractDeviceAPIEncapsulator(EDeviceAPI::DirectX11, owner, pp) {

    _wrapper.Create(this, windowHandle, Parameters());
    _writer = new FDX11ConstantWriter(this);
}
//----------------------------------------------------------------------------
FDX11DeviceAPIEncapsulator::~FDX11DeviceAPIEncapsulator() {
    _writer = nullptr;

    _wrapper.Destroy(this);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::Reset(const FPresentationParameters& pp) {
    UNUSED(pp);
    // TODO
    AssertNotImplemented();
    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::Present() {
    const UINT syncInterval = UINT(Parameters().PresentationInterval());
    _wrapper.SwapChain()->Present(syncInterval, 0);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::ClearState() {
    _wrapper.ImmediateContext()->ClearState();
    _wrapper.ImmediateContext()->Flush();

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Alpha/Raster/Depth State
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetViewport(const ViewportF& viewport) {
    ::D3D11_VIEWPORT dx11Viewport;

    dx11Viewport.TopLeftX = viewport.Left();
    dx11Viewport.TopLeftY = viewport.Top();

    dx11Viewport.Width = viewport.Width();
    dx11Viewport.Height = viewport.Height();

    dx11Viewport.MinDepth = viewport.Near();
    dx11Viewport.MaxDepth = viewport.Far();

    _wrapper.ImmediateContext()->RSSetViewports(1, &dx11Viewport);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetViewports(const TMemoryView<const ViewportF>& viewports) {
    const UINT dx11NumViews = checked_cast<UINT>(viewports.size());
    STACKLOCAL_POD_ARRAY(::D3D11_VIEWPORT, dx11Viewports, viewports.size());

    for (size_t i = 0; i < dx11NumViews; ++i) {
        const ViewportF& viewport = viewports[i];
        ::D3D11_VIEWPORT& dx11Viewport = dx11Viewports[i];

        dx11Viewport.TopLeftX = viewport.Left();
        dx11Viewport.TopLeftY = viewport.Top();

        dx11Viewport.Width = viewport.Width();
        dx11Viewport.Height = viewport.Height();

        dx11Viewport.MinDepth = viewport.Near();
        dx11Viewport.MaxDepth = viewport.Far();
    }

    _wrapper.ImmediateContext()->RSSetViewports(dx11NumViews, dx11Viewports.Pointer());

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
FDeviceAPIDependantBlendState *FDX11DeviceAPIEncapsulator::CreateBlendState(FBlendState *state) {
    return new FDX11BlendState(this, state);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetBlendState(const FBlendState *state) {
    ::ID3D11BlendState *const dx11BlendState =
        checked_cast<const FDX11BlendState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->OMSetBlendState(
        dx11BlendState,
        state->BlendFactor()._data,
        state->MultiSampleMask()
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyBlendState(FBlendState * /* state */, PDeviceAPIDependantBlendState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantRasterizerState *FDX11DeviceAPIEncapsulator::CreateRasterizerState(FRasterizerState *state) {
    return new FDX11RasterizerState(this, state);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetRasterizerState(const FRasterizerState *state) {
    ::ID3D11RasterizerState *const dx11RasterizerState =
        checked_cast<const FDX11RasterizerState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->RSSetState(dx11RasterizerState);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyRasterizerState(FRasterizerState * /* state */, PDeviceAPIDependantRasterizerState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantDepthStencilState *FDX11DeviceAPIEncapsulator::CreateDepthStencilState(FDepthStencilState *state) {
    return new FDX11DepthStencilState(this, state);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetDepthStencilState(const FDepthStencilState *state) {
    ::ID3D11DepthStencilState *const dx11DepthStencilState =
        checked_cast<const FDX11DepthStencilState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->OMSetDepthStencilState(
        dx11DepthStencilState,
        state->RendererStencil()
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyDepthStencilState(FDepthStencilState * /* state */, PDeviceAPIDependantDepthStencilState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantSamplerState *FDX11DeviceAPIEncapsulator::CreateSamplerState(FSamplerState *state) {
    return new FDX11SamplerState(this, state);
}
//---------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetSamplerState(EShaderProgramType stage, size_t slot, const FSamplerState *state) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11SamplerState *const dx11SamplerState =
        checked_cast<const FDX11SamplerState *>(state->DeviceAPIDependantState().get())->Entity();

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case EShaderProgramType::Vertex:
        context->VSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case EShaderProgramType::Hull:
        context->HSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case EShaderProgramType::Domain:
        context->DSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case EShaderProgramType::Pixel:
        context->PSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case EShaderProgramType::Geometry:
        context->GSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case EShaderProgramType::Compute:
        context->CSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}

//---------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetSamplerStates(EShaderProgramType stage, const TMemoryView<const FSamplerState *>& states) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11SamplerState *dx11SamplerStates[16];

    forrange(i, 0, states.size())
        dx11SamplerStates[i] = checked_cast<const FDX11SamplerState *>(states[i]->DeviceAPIDependantState().get())->Entity();

    const UINT dx11Slot = 0;
    const UINT dx11NumBuffers = checked_cast<UINT>(states.size());

    switch (stage)
    {
    case EShaderProgramType::Vertex:
        context->VSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case EShaderProgramType::Hull:
        context->HSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case EShaderProgramType::Domain:
        context->DSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case EShaderProgramType::Pixel:
        context->PSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case EShaderProgramType::Geometry:
        context->GSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case EShaderProgramType::Compute:
        context->CSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//---------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroySamplerState(FSamplerState * /* state */, PDeviceAPIDependantSamplerState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Index/Vertex Buffer
//----------------------------------------------------------------------------
FDeviceAPIDependantVertexDeclaration *FDX11DeviceAPIEncapsulator::CreateVertexDeclaration(FVertexDeclaration *declaration) {
    return new FDX11VertexDeclaration(this, declaration);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyVertexDeclaration(FVertexDeclaration * /* declaration */, PDeviceAPIDependantVertexDeclaration& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantResourceBuffer *FDX11DeviceAPIEncapsulator::CreateIndexBuffer(IndexBuffer *indexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) {
    return new FDX11ResourceBuffer(this, indexBuffer, resourceBuffer, optionalData);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer) {
    SetIndexBuffer(indexBuffer, 0);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) {
    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(indexBuffer->Buffer());

    _wrapper.ImmediateContext()->IASetIndexBuffer(
        dx11Buffer,
        indexBuffer->IndexElementSize() == EIndexElementSize::ThirtyTwoBits
            ? DXGI_FORMAT_R32_UINT
            : DXGI_FORMAT_R16_UINT,
            checked_cast<UINT>(offset)
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyIndexBuffer(IndexBuffer * /* indexBuffer */, PDeviceAPIDependantResourceBuffer& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantResourceBuffer *FDX11DeviceAPIEncapsulator::CreateVertexBuffer(FVertexBuffer *vertexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) {
    return new FDX11ResourceBuffer(this, vertexBuffer, resourceBuffer, optionalData);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetVertexBuffer(const FVertexBuffer *vertexBuffer) {
    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(vertexBuffer->Buffer());

    const UINT stride = checked_cast<UINT>(vertexBuffer->VertexDeclaration()->SizeInBytes());
    const UINT offset = 0;

    _wrapper.ImmediateContext()->IASetVertexBuffers(0, 1, &dx11Buffer, &stride, &offset);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetVertexBuffer(const FVertexBuffer *vertexBuffer, u32 vertexOffset) {
    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(vertexBuffer->Buffer());

    const UINT stride = checked_cast<UINT>(vertexBuffer->VertexDeclaration()->SizeInBytes());
    const UINT offset = checked_cast<UINT>(vertexOffset);

    _wrapper.ImmediateContext()->IASetVertexBuffers(0, 1, &dx11Buffer, &stride, &offset);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetVertexBuffer(const TMemoryView<const FVertexBufferBinding>& bindings) {
    const size_t k = bindings.size();

    STACKLOCAL_POD_ARRAY(::ID3D11Buffer*, dx11Buffers, k);
    STACKLOCAL_POD_ARRAY(UINT, strideAndOffset, k*2);

    for (size_t i = 0; i < k; ++i) {
        const FVertexBufferBinding& binding = bindings[i];

        dx11Buffers[i] = DX11DeviceBufferEntity(binding.VertexBuffer->Buffer());
        strideAndOffset[i] = checked_cast<UINT>(binding.VertexBuffer->Buffer().SizeInBytes());
        strideAndOffset[i + k] = checked_cast<UINT>(binding.VertexOffset);
    }

    _wrapper.ImmediateContext()->IASetVertexBuffers(
        0,
        checked_cast<UINT>(k),
        dx11Buffers.Pointer(),
        strideAndOffset.Pointer(),
        strideAndOffset.Pointer() + k
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyVertexBuffer(FVertexBuffer * /* buffer */, PDeviceAPIDependantResourceBuffer& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Shaders
//----------------------------------------------------------------------------
const FDeviceAPIDependantConstantWriter *FDX11DeviceAPIEncapsulator::ConstantWriter() const {
    return _writer.get();
}
//----------------------------------------------------------------------------
FDeviceAPIDependantResourceBuffer *FDX11DeviceAPIEncapsulator::CreateConstantBuffer(FConstantBuffer *constantBuffer, FDeviceResourceBuffer *resourceBuffer) {
    return new FDX11ResourceBuffer(this, constantBuffer, resourceBuffer, TMemoryView<const u8>());
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetConstantBuffer(EShaderProgramType stage, size_t slot, const FConstantBuffer *constantBuffer) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(constantBuffer->Buffer());

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case EShaderProgramType::Vertex:
        context->VSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case EShaderProgramType::Hull:
        context->HSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case EShaderProgramType::Domain:
        context->DSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case EShaderProgramType::Pixel:
        context->PSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case EShaderProgramType::Geometry:
        context->GSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case EShaderProgramType::Compute:
        context->CSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetConstantBuffers(EShaderProgramType stage, const TMemoryView<const FConstantBuffer *>& constantBuffers) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11Buffer *dx11Buffers[14];
    AssertRelease(constantBuffers.size() <= lengthof(dx11Buffers));

    forrange(i, 0, constantBuffers.size())
        dx11Buffers[i] = DX11DeviceBufferEntity(constantBuffers[i]->Buffer());

    const UINT dx11Slot = 0;
    const UINT dx11NumBuffers = checked_cast<UINT>(constantBuffers.size());

    switch (stage)
    {
    case EShaderProgramType::Vertex:
        context->VSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case EShaderProgramType::Hull:
        context->HSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case EShaderProgramType::Domain:
        context->DSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case EShaderProgramType::Pixel:
        context->PSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case EShaderProgramType::Geometry:
        context->GSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case EShaderProgramType::Compute:
        context->CSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyConstantBuffer(FConstantBuffer * /* constantBuffer */, PDeviceAPIDependantResourceBuffer& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantShaderProgram* FDX11DeviceAPIEncapsulator::CreateShaderProgram(FShaderProgram* program) {
    return new FDX11ShaderProgram(this, program);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyShaderProgram(FShaderProgram* /* program */, PDeviceAPIDependantShaderProgram& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantShaderEffect *FDX11DeviceAPIEncapsulator::CreateShaderEffect(FShaderEffect *effect) {
    return new FDX11ShaderEffect(this, effect);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetShaderEffect(const FShaderEffect *effect) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    const FDX11ShaderEffect *dx11Effect = checked_cast<FDX11ShaderEffect *>(effect->DeviceAPIDependantEffect().get());

    context->IASetInputLayout(dx11Effect->InputLayout());

    context->VSSetShader(dx11Effect->VertexShader(), NULL, 0);
    context->DSSetShader(dx11Effect->DomainShader(), NULL, 0);
    context->HSSetShader(dx11Effect->HullShader(), NULL, 0);
    context->GSSetShader(dx11Effect->GeometryShader(), NULL, 0);
    context->PSSetShader(dx11Effect->PixelShader(), NULL, 0);
    context->CSSetShader(dx11Effect->ComputeShader(), NULL, 0);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyShaderEffect(FShaderEffect * /* effect */, PDeviceAPIDependantShaderEffect& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Textures
//---------------------------------------------------------------------------
FDeviceAPIDependantTexture2D *FDX11DeviceAPIEncapsulator::CreateTexture2D(FTexture2D *texture, const TMemoryView<const u8>& optionalData) {
    return new FDX11Texture2D(this, texture, optionalData);
}
//---------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyTexture2D(FTexture2D * /* texture */, PDeviceAPIDependantTexture2D& entity) {
    RemoveRef_AssertReachZero(entity);
}
//---------------------------------------------------------------------------
FDeviceAPIDependantTextureCube *FDX11DeviceAPIEncapsulator::CreateTextureCube(FTextureCube *texture, const TMemoryView<const u8>& optionalData) {
    return new FDX11TextureCube(this, texture, optionalData);
}
//---------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyTextureCube(FTextureCube * /* texture */, PDeviceAPIDependantTextureCube& entity) {
    RemoveRef_AssertReachZero(entity);
}
//---------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetTexture(EShaderProgramType stage, size_t slot, const FTexture *texture) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11ShaderResourceView *dx11ResourceView = nullptr;

    if (texture) {
        Assert(texture->Frozen());
        Assert(texture->Available());

        const IDeviceAPIDependantAbstractTextureContent *content = texture->TextureEntity()->Content();
        if (!content)
            CORE_THROW_IT(FDeviceEncapsulatorException("DX11: empty texture content could not be set to device", this, texture));

        const FDX11AbstractTextureContent *dx11Content = checked_cast<const FDX11AbstractTextureContent *>(content);

        dx11ResourceView = dx11Content->ShaderView();
        AssertRelease(dx11ResourceView);
    }

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case EShaderProgramType::Vertex:
        context->VSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case EShaderProgramType::Hull:
        context->HSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case EShaderProgramType::Domain:
        context->DSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case EShaderProgramType::Pixel:
        context->PSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case EShaderProgramType::Geometry:
        context->GSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case EShaderProgramType::Compute:
        context->CSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//---------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetTextures(EShaderProgramType stage, const TMemoryView<const FTexture *>& textures) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11ShaderResourceView *dx11ResourceViews[16];
    AssertRelease(textures.size() <= lengthof(dx11ResourceViews));

    forrange(i, 0, textures.size()) {
        const FTexture *texture = textures[i];
        if (texture) {
            Assert(texture->Frozen());
            Assert(texture->Available());

            const IDeviceAPIDependantAbstractTextureContent *content = texture->TextureEntity()->Content();
            if (!content)
                CORE_THROW_IT(FDeviceEncapsulatorException("DX11: empty texture content could not be set to device", this, texture));

            const FDX11AbstractTextureContent *dx11Content = checked_cast<const FDX11AbstractTextureContent *>(content);

            dx11ResourceViews[i] = dx11Content->ShaderView();
        }
        else {
            dx11ResourceViews[i] = nullptr;
        }
    }

    const UINT dx11Slot = 0;
    const UINT dx11NumBuffers = checked_cast<UINT>(textures.size());

    switch (stage)
    {
    case EShaderProgramType::Vertex:
        context->VSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case EShaderProgramType::Hull:
        context->HSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case EShaderProgramType::Domain:
        context->DSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case EShaderProgramType::Pixel:
        context->PSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case EShaderProgramType::Geometry:
        context->GSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case EShaderProgramType::Compute:
        context->CSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Render Targets
//----------------------------------------------------------------------------
FRenderTarget *FDX11DeviceAPIEncapsulator::BackBufferRenderTarget() {
    return _wrapper.BackBufferRenderTarget().get();
}
//----------------------------------------------------------------------------
FDepthStencil *FDX11DeviceAPIEncapsulator::BackBufferDepthStencil() {
    return _wrapper.BackBufferDepthStencil().get();
}
//----------------------------------------------------------------------------
const FRenderTarget *FDX11DeviceAPIEncapsulator::BackBufferRenderTarget() const {
    return _wrapper.BackBufferRenderTarget().get();
}
//----------------------------------------------------------------------------
const FDepthStencil *FDX11DeviceAPIEncapsulator::BackBufferDepthStencil() const {
    return _wrapper.BackBufferDepthStencil().get();
}
//----------------------------------------------------------------------------
FDeviceAPIDependantRenderTarget *FDX11DeviceAPIEncapsulator::CreateRenderTarget(FRenderTarget *renderTarget, const TMemoryView<const u8>& optionalData) {
    return new FDX11RenderTarget(this, renderTarget, optionalData);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetRenderTarget(const FRenderTarget *renderTarget, const FDepthStencil *depthStencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11RenderTargetView *dx11RenderTarget = nullptr;
    if (renderTarget) {
        dx11RenderTarget = checked_cast<const FDX11RenderTarget *>(renderTarget->DeviceAPIDependantTexture2D().get())->RenderTargetView();
        Assert(dx11RenderTarget);
    }

    ::ID3D11DepthStencilView *dx11DepthStencil = nullptr;
    if (depthStencil) {
        dx11DepthStencil = checked_cast<const FDX11DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
        Assert(dx11DepthStencil);
    }

    context->OMSetRenderTargets(1, &dx11RenderTarget, dx11DepthStencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetRenderTargets(const TMemoryView<const FRenderTargetBinding>& bindings, const FDepthStencil *depthStencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    STACKLOCAL_POD_ARRAY(::ID3D11RenderTargetView*, dx11RenderTargets, bindings.size());
    ::SecureZeroMemory(dx11RenderTargets.Pointer(), dx11RenderTargets.SizeInBytes());

    for (const FRenderTargetBinding& binding : bindings) {
        Assert(!dx11RenderTargets[binding.Slot]);
        dx11RenderTargets[binding.Slot] = checked_cast<const FDX11RenderTarget *>(binding.RT->DeviceAPIDependantTexture2D().get())->RenderTargetView();
    }

    ::ID3D11DepthStencilView *dx11DepthStencil = nullptr;
    if (depthStencil) {
        dx11DepthStencil = checked_cast<const FDX11DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
        Assert(dx11DepthStencil);
    }

    UINT numViews = checked_cast<UINT>(bindings.size());

    context->OMSetRenderTargets(numViews, dx11RenderTargets.Pointer(), dx11DepthStencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyRenderTarget(FRenderTarget * /* renderTarget */, PDeviceAPIDependantRenderTarget& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantDepthStencil *FDX11DeviceAPIEncapsulator::CreateDepthStencil(FDepthStencil *depthStencil, const TMemoryView<const u8>& optionalData) {
    return new FDX11DepthStencil(this, depthStencil, optionalData);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DestroyDepthStencil(FDepthStencil * /* depthStencil */, PDeviceAPIDependantDepthStencil& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::Clear(const FRenderTarget *renderTarget, const FLinearColor& color) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11RenderTargetView *dx11RenderTarget =
        checked_cast<const FDX11RenderTarget *>(renderTarget->DeviceAPIDependantTexture2D().get())->RenderTargetView();
    Assert(dx11RenderTarget);

    const FLOAT dx11ColorRGBA[4] = {color.R, color.G, color.B, color.A};

    context->ClearRenderTargetView(dx11RenderTarget, dx11ColorRGBA);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::Clear(const FDepthStencil *depthStencil, EClearOptions opts, float depth, u8 stencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11DepthStencilView *dx11DepthStencil =
        checked_cast<const FDX11DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
    Assert(dx11DepthStencil);

    UINT dx11ClearFlags = 0;
    if (opts ^ EClearOptions::Depth)
        dx11ClearFlags |= D3D11_CLEAR_DEPTH;
    if (opts ^ EClearOptions::Stencil)
        dx11ClearFlags |= D3D11_CLEAR_STENCIL;

    context->ClearDepthStencilView(dx11DepthStencil, dx11ClearFlags, depth, stencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Draw
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DrawPrimitives(EPrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();
    ::D3D11_PRIMITIVE_TOPOLOGY dx11PrimitiveTopology = PrimitiveTypeToDX11PrimitiveTopology(primitiveType);

    context->IASetPrimitiveTopology(dx11PrimitiveTopology);
    context->Draw(
        checked_cast<UINT>(IndexCount(primitiveType, primitiveCount)),
        checked_cast<UINT>(startVertex));

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DrawIndexedPrimitives(EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();
    ::D3D11_PRIMITIVE_TOPOLOGY dx11PrimitiveTopology = PrimitiveTypeToDX11PrimitiveTopology(primitiveType);

    context->IASetPrimitiveTopology(dx11PrimitiveTopology);
    context->DrawIndexed(
        checked_cast<UINT>(IndexCount(primitiveType, primitiveCount)),
        checked_cast<UINT>(startIndex),
        checked_cast<UINT>(baseVertex));

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::DrawInstancedPrimitives(EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();
    ::D3D11_PRIMITIVE_TOPOLOGY dx11PrimitiveTopology = PrimitiveTypeToDX11PrimitiveTopology(primitiveType);

    context->IASetPrimitiveTopology(dx11PrimitiveTopology);
    context->DrawIndexedInstanced(
        checked_cast<UINT>(IndexCount(primitiveType, primitiveCount)),
        checked_cast<UINT>(instanceCount),
        checked_cast<UINT>(startIndex),
        checked_cast<UINT>(baseVertex),
        checked_cast<UINT>(startInstance));

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Diagnostics
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
bool FDX11DeviceAPIEncapsulator::IsProfilerAttached() const {
#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    return  nullptr != _wrapper.UserDefinedAnnotation()/* &&
            _wrapper.UserDefinedAnnotation()->GetStatus() */;
#else
    return false;
#endif
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::BeginEvent(const wchar_t *name) {
    Assert(_wrapper.UserDefinedAnnotation());

    _wrapper.UserDefinedAnnotation()->BeginEvent(name);
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::EndEvent() {
    Assert(_wrapper.UserDefinedAnnotation());

    _wrapper.UserDefinedAnnotation()->EndEvent();
}
//----------------------------------------------------------------------------
void FDX11DeviceAPIEncapsulator::SetMarker(const wchar_t *name) {
    Assert(_wrapper.UserDefinedAnnotation());

    _wrapper.UserDefinedAnnotation()->SetMarker(name);
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
