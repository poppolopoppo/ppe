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
#include "Core/Maths/Geometry/ScalarRectangle.h"
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
const DX11DeviceWrapper *DX11GetDeviceWrapper(const IDeviceAPIEncapsulator *device) {
    return &checked_cast<const DX11DeviceAPIEncapsulator *>(device->APIEncapsulator())->Wrapper();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11DeviceAPIEncapsulator::DX11DeviceAPIEncapsulator(DeviceEncapsulator *owner, void *windowHandle, const PresentationParameters& pp)
:   AbstractDeviceAPIEncapsulator(DeviceAPI::DirectX11, owner, pp) {

    _wrapper.Create(this, windowHandle, Parameters());
    _writer = new DX11ConstantWriter(this);
}
//----------------------------------------------------------------------------
DX11DeviceAPIEncapsulator::~DX11DeviceAPIEncapsulator() {
    _writer = nullptr;

    _wrapper.Destroy(this);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::Reset(const PresentationParameters& pp) {
    UNUSED(pp);
    // TODO
    AssertNotImplemented();
    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::Present() {
    const UINT syncInterval = UINT(Parameters().PresentationInterval());
    _wrapper.SwapChain()->Present(syncInterval, 0);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::ClearState() {
    _wrapper.ImmediateContext()->ClearState();
    _wrapper.ImmediateContext()->Flush();

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Alpha/Raster/Depth State
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetViewport(const ViewportF& viewport) {
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
void DX11DeviceAPIEncapsulator::SetViewports(const MemoryView<const ViewportF>& viewports) {
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
DeviceAPIDependantBlendState *DX11DeviceAPIEncapsulator::CreateBlendState(BlendState *state) {
    return new DX11BlendState(this, state);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetBlendState(const BlendState *state) {
    ::ID3D11BlendState *const dx11BlendState =
        checked_cast<const DX11BlendState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->OMSetBlendState(
        dx11BlendState,
        state->BlendFactor()._data,
        state->MultiSampleMask()
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyBlendState(BlendState * /* state */, PDeviceAPIDependantBlendState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantRasterizerState *DX11DeviceAPIEncapsulator::CreateRasterizerState(RasterizerState *state) {
    return new DX11RasterizerState(this, state);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetRasterizerState(const RasterizerState *state) {
    ::ID3D11RasterizerState *const dx11RasterizerState =
        checked_cast<const DX11RasterizerState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->RSSetState(dx11RasterizerState);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyRasterizerState(RasterizerState * /* state */, PDeviceAPIDependantRasterizerState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencilState *DX11DeviceAPIEncapsulator::CreateDepthStencilState(DepthStencilState *state) {
    return new DX11DepthStencilState(this, state);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetDepthStencilState(const DepthStencilState *state) {
    ::ID3D11DepthStencilState *const dx11DepthStencilState =
        checked_cast<const DX11DepthStencilState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->OMSetDepthStencilState(
        dx11DepthStencilState,
        state->RendererStencil()
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyDepthStencilState(DepthStencilState * /* state */, PDeviceAPIDependantDepthStencilState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantSamplerState *DX11DeviceAPIEncapsulator::CreateSamplerState(SamplerState *state) {
    return new DX11SamplerState(this, state);
}
//---------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetSamplerState(ShaderProgramType stage, size_t slot, const SamplerState *state) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11SamplerState *const dx11SamplerState =
        checked_cast<const DX11SamplerState *>(state->DeviceAPIDependantState().get())->Entity();

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case ShaderProgramType::Vertex:
        context->VSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case ShaderProgramType::Hull:
        context->HSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case ShaderProgramType::Domain:
        context->DSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case ShaderProgramType::Pixel:
        context->PSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case ShaderProgramType::Geometry:
        context->GSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case ShaderProgramType::Compute:
        context->CSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}

//---------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetSamplerStates(ShaderProgramType stage, const MemoryView<const SamplerState *>& states) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11SamplerState *dx11SamplerStates[16];

    forrange(i, 0, states.size())
        dx11SamplerStates[i] = checked_cast<const DX11SamplerState *>(states[i]->DeviceAPIDependantState().get())->Entity();

    const UINT dx11Slot = 0;
    const UINT dx11NumBuffers = checked_cast<UINT>(states.size());

    switch (stage)
    {
    case ShaderProgramType::Vertex:
        context->VSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case ShaderProgramType::Hull:
        context->HSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case ShaderProgramType::Domain:
        context->DSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case ShaderProgramType::Pixel:
        context->PSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case ShaderProgramType::Geometry:
        context->GSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    case ShaderProgramType::Compute:
        context->CSSetSamplers(dx11Slot, dx11NumBuffers, dx11SamplerStates);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//---------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroySamplerState(SamplerState * /* state */, PDeviceAPIDependantSamplerState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Index/Vertex Buffer
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration *DX11DeviceAPIEncapsulator::CreateVertexDeclaration(VertexDeclaration *declaration) {
    return new DX11VertexDeclaration(this, declaration);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyVertexDeclaration(VertexDeclaration * /* declaration */, PDeviceAPIDependantVertexDeclaration& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DX11DeviceAPIEncapsulator::CreateIndexBuffer(IndexBuffer *indexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    return new DX11ResourceBuffer(this, indexBuffer, resourceBuffer, optionalData);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer) {
    SetIndexBuffer(indexBuffer, 0);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) {
    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(indexBuffer->Buffer());

    _wrapper.ImmediateContext()->IASetIndexBuffer(
        dx11Buffer,
        indexBuffer->IndexElementSize() == IndexElementSize::ThirtyTwoBits
            ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT,
        checked_cast<UINT>(offset)
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyIndexBuffer(IndexBuffer * /* indexBuffer */, PDeviceAPIDependantResourceBuffer& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DX11DeviceAPIEncapsulator::CreateVertexBuffer(VertexBuffer *vertexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    return new DX11ResourceBuffer(this, vertexBuffer, resourceBuffer, optionalData);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetVertexBuffer(const VertexBuffer *vertexBuffer) {
    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(vertexBuffer->Buffer());

    const UINT stride = checked_cast<UINT>(vertexBuffer->VertexDeclaration()->SizeInBytes());
    const UINT offset = 0;

    _wrapper.ImmediateContext()->IASetVertexBuffers(0, 1, &dx11Buffer, &stride, &offset);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetVertexBuffer(const VertexBuffer *vertexBuffer, u32 vertexOffset) {
    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(vertexBuffer->Buffer());

    const UINT stride = checked_cast<UINT>(vertexBuffer->VertexDeclaration()->SizeInBytes());
    const UINT offset = checked_cast<UINT>(vertexOffset);

    _wrapper.ImmediateContext()->IASetVertexBuffers(0, 1, &dx11Buffer, &stride, &offset);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetVertexBuffer(const MemoryView<const VertexBufferBinding>& bindings) {
    const size_t k = bindings.size();

    STACKLOCAL_POD_ARRAY(::ID3D11Buffer*, dx11Buffers, k);
    STACKLOCAL_POD_ARRAY(UINT, strideAndOffset, k*2);

    for (size_t i = 0; i < k; ++i) {
        const VertexBufferBinding& binding = bindings[i];

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
void DX11DeviceAPIEncapsulator::DestroyVertexBuffer(VertexBuffer * /* buffer */, PDeviceAPIDependantResourceBuffer& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Shaders
//----------------------------------------------------------------------------
const DeviceAPIDependantConstantWriter *DX11DeviceAPIEncapsulator::ConstantWriter() const {
    return _writer.get();
}
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DX11DeviceAPIEncapsulator::CreateConstantBuffer(ConstantBuffer *constantBuffer, DeviceResourceBuffer *resourceBuffer) {
    return new DX11ResourceBuffer(this, constantBuffer, resourceBuffer, MemoryView<const u8>());
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetConstantBuffer(ShaderProgramType stage, size_t slot, const ConstantBuffer *constantBuffer) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(constantBuffer->Buffer());

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case ShaderProgramType::Vertex:
        context->VSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case ShaderProgramType::Hull:
        context->HSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case ShaderProgramType::Domain:
        context->DSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case ShaderProgramType::Pixel:
        context->PSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case ShaderProgramType::Geometry:
        context->GSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case ShaderProgramType::Compute:
        context->CSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetConstantBuffers(ShaderProgramType stage, const MemoryView<const ConstantBuffer *>& constantBuffers) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11Buffer *dx11Buffers[14];
    AssertRelease(constantBuffers.size() <= lengthof(dx11Buffers));

    forrange(i, 0, constantBuffers.size())
        dx11Buffers[i] = DX11DeviceBufferEntity(constantBuffers[i]->Buffer());

    const UINT dx11Slot = 0;
    const UINT dx11NumBuffers = checked_cast<UINT>(constantBuffers.size());

    switch (stage)
    {
    case ShaderProgramType::Vertex:
        context->VSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case ShaderProgramType::Hull:
        context->HSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case ShaderProgramType::Domain:
        context->DSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case ShaderProgramType::Pixel:
        context->PSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case ShaderProgramType::Geometry:
        context->GSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    case ShaderProgramType::Compute:
        context->CSSetConstantBuffers(dx11Slot, dx11NumBuffers, dx11Buffers);
        break;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyConstantBuffer(ConstantBuffer * /* constantBuffer */, PDeviceAPIDependantResourceBuffer& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderProgram* DX11DeviceAPIEncapsulator::CreateShaderProgram(ShaderProgram* program) {
    return new DX11ShaderProgram(this, program);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyShaderProgram(ShaderProgram* /* program */, PDeviceAPIDependantShaderProgram& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect *DX11DeviceAPIEncapsulator::CreateShaderEffect(ShaderEffect *effect) {
    return new DX11ShaderEffect(this, effect);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetShaderEffect(const ShaderEffect *effect) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    const DX11ShaderEffect *dx11Effect = checked_cast<DX11ShaderEffect *>(effect->DeviceAPIDependantEffect().get());

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
void DX11DeviceAPIEncapsulator::DestroyShaderEffect(ShaderEffect * /* effect */, PDeviceAPIDependantShaderEffect& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Textures
//---------------------------------------------------------------------------
DeviceAPIDependantTexture2D *DX11DeviceAPIEncapsulator::CreateTexture2D(Texture2D *texture, const MemoryView<const u8>& optionalData) {
    return new DX11Texture2D(this, texture, optionalData);
}
//---------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyTexture2D(Texture2D * /* texture */, PDeviceAPIDependantTexture2D& entity) {
    RemoveRef_AssertReachZero(entity);
}
//---------------------------------------------------------------------------
DeviceAPIDependantTextureCube *DX11DeviceAPIEncapsulator::CreateTextureCube(TextureCube *texture, const MemoryView<const u8>& optionalData) {
    return new DX11TextureCube(this, texture, optionalData);
}
//---------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyTextureCube(TextureCube * /* texture */, PDeviceAPIDependantTextureCube& entity) {
    RemoveRef_AssertReachZero(entity);
}
//---------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetTexture(ShaderProgramType stage, size_t slot, const Texture *texture) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11ShaderResourceView *dx11ResourceView = nullptr;

    if (texture) {
        Assert(texture->Frozen());
        Assert(texture->Available());

        const IDeviceAPIDependantAbstractTextureContent *content = texture->TextureEntity()->Content();
        if (!content)
            throw DeviceEncapsulatorException("DX11: empty texture content could not be set to device", this, texture);

        const DX11AbstractTextureContent *dx11Content = checked_cast<const DX11AbstractTextureContent *>(content);

        dx11ResourceView = dx11Content->ShaderView();
        AssertRelease(dx11ResourceView);
    }

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case ShaderProgramType::Vertex:
        context->VSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case ShaderProgramType::Hull:
        context->HSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case ShaderProgramType::Domain:
        context->DSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case ShaderProgramType::Pixel:
        context->PSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case ShaderProgramType::Geometry:
        context->GSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case ShaderProgramType::Compute:
        context->CSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//---------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetTextures(ShaderProgramType stage, const MemoryView<const Texture *>& textures) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11ShaderResourceView *dx11ResourceViews[16];
    AssertRelease(textures.size() <= lengthof(dx11ResourceViews));

    forrange(i, 0, textures.size()) {
        const Texture *texture = textures[i];
        if (texture) {
            Assert(texture->Frozen());
            Assert(texture->Available());

            const IDeviceAPIDependantAbstractTextureContent *content = texture->TextureEntity()->Content();
            if (!content)
                throw DeviceEncapsulatorException("DX11: empty texture content could not be set to device", this, texture);

            const DX11AbstractTextureContent *dx11Content = checked_cast<const DX11AbstractTextureContent *>(content);

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
    case ShaderProgramType::Vertex:
        context->VSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case ShaderProgramType::Hull:
        context->HSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case ShaderProgramType::Domain:
        context->DSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case ShaderProgramType::Pixel:
        context->PSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case ShaderProgramType::Geometry:
        context->GSSetShaderResources(dx11Slot, dx11NumBuffers, dx11ResourceViews);
        break;
    case ShaderProgramType::Compute:
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
RenderTarget *DX11DeviceAPIEncapsulator::BackBufferRenderTarget() {
    return _wrapper.BackBufferRenderTarget().get();
}
//----------------------------------------------------------------------------
DepthStencil *DX11DeviceAPIEncapsulator::BackBufferDepthStencil() {
    return _wrapper.BackBufferDepthStencil().get();
}
//----------------------------------------------------------------------------
const RenderTarget *DX11DeviceAPIEncapsulator::BackBufferRenderTarget() const {
    return _wrapper.BackBufferRenderTarget().get();
}
//----------------------------------------------------------------------------
const DepthStencil *DX11DeviceAPIEncapsulator::BackBufferDepthStencil() const {
    return _wrapper.BackBufferDepthStencil().get();
}
//----------------------------------------------------------------------------
DeviceAPIDependantRenderTarget *DX11DeviceAPIEncapsulator::CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) {
    return new DX11RenderTarget(this, renderTarget, optionalData);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetRenderTarget(const RenderTarget *renderTarget, const DepthStencil *depthStencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11RenderTargetView *dx11RenderTarget = nullptr;
    if (renderTarget) {
        dx11RenderTarget = checked_cast<const DX11RenderTarget *>(renderTarget->DeviceAPIDependantTexture2D().get())->RenderTargetView();
        Assert(dx11RenderTarget);
    }

    ::ID3D11DepthStencilView *dx11DepthStencil = nullptr;
    if (depthStencil) {
        dx11DepthStencil = checked_cast<const DX11DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
        Assert(dx11DepthStencil);
    }

    context->OMSetRenderTargets(1, &dx11RenderTarget, dx11DepthStencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const DepthStencil *depthStencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    STACKLOCAL_POD_ARRAY(::ID3D11RenderTargetView*, dx11RenderTargets, bindings.size());
    ::SecureZeroMemory(dx11RenderTargets.Pointer(), dx11RenderTargets.SizeInBytes());

    for (const RenderTargetBinding& binding : bindings) {
        Assert(!dx11RenderTargets[binding.Slot]);
        dx11RenderTargets[binding.Slot] = checked_cast<const DX11RenderTarget *>(binding.RT->DeviceAPIDependantTexture2D().get())->RenderTargetView();
    }

    ::ID3D11DepthStencilView *dx11DepthStencil = nullptr;
    if (depthStencil) {
        dx11DepthStencil = checked_cast<const DX11DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
        Assert(dx11DepthStencil);
    }

    UINT numViews = checked_cast<UINT>(bindings.size());

    context->OMSetRenderTargets(numViews, dx11RenderTargets.Pointer(), dx11DepthStencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyRenderTarget(RenderTarget * /* renderTarget */, PDeviceAPIDependantRenderTarget& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencil *DX11DeviceAPIEncapsulator::CreateDepthStencil(DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) {
    return new DX11DepthStencil(this, depthStencil, optionalData);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DestroyDepthStencil(DepthStencil * /* depthStencil */, PDeviceAPIDependantDepthStencil& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::Clear(const RenderTarget *renderTarget, const ColorRGBAF& color) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11RenderTargetView *dx11RenderTarget =
        checked_cast<const DX11RenderTarget *>(renderTarget->DeviceAPIDependantTexture2D().get())->RenderTargetView();
    Assert(dx11RenderTarget);

    const FLOAT dx11ColorRGBA[4] = {color.r(), color.g(), color.b(), color.a()};

    context->ClearRenderTargetView(dx11RenderTarget, dx11ColorRGBA);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::Clear(const DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11DepthStencilView *dx11DepthStencil =
        checked_cast<const DX11DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
    Assert(dx11DepthStencil);

    UINT dx11ClearFlags = 0;
    if (Meta::HasFlag(opts, ClearOptions::Depth) )
        dx11ClearFlags |= D3D11_CLEAR_DEPTH;
    if (Meta::HasFlag(opts, ClearOptions::Stencil) )
        dx11ClearFlags |= D3D11_CLEAR_STENCIL;

    context->ClearDepthStencilView(dx11DepthStencil, dx11ClearFlags, depth, stencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Draw
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DrawPrimitives(PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();
    ::D3D11_PRIMITIVE_TOPOLOGY dx11PrimitiveTopology = PrimitiveTypeToDX11PrimitiveTopology(primitiveType);

    context->IASetPrimitiveTopology(dx11PrimitiveTopology);
    context->Draw(
        checked_cast<UINT>(IndexCount(primitiveType, primitiveCount)),
        checked_cast<UINT>(startVertex));

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::DrawIndexedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) {
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
void DX11DeviceAPIEncapsulator::DrawInstancedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) {
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
bool DX11DeviceAPIEncapsulator::IsProfilerAttached() const {
#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    return  nullptr != _wrapper.UserDefinedAnnotation()/* &&
            _wrapper.UserDefinedAnnotation()->GetStatus() */;
#else
    return false;
#endif
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::BeginEvent(const wchar_t *name) {
    Assert(_wrapper.UserDefinedAnnotation());

    _wrapper.UserDefinedAnnotation()->BeginEvent(name);
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::EndEvent() {
    Assert(_wrapper.UserDefinedAnnotation());

    _wrapper.UserDefinedAnnotation()->EndEvent();
}
//----------------------------------------------------------------------------
void DX11DeviceAPIEncapsulator::SetMarker(const wchar_t *name) {
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
