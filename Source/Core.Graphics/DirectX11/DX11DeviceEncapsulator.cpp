#include "stdafx.h"

#include "DX11DeviceEncapsulator.h"

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

#include "DX11DeviceWrapper.h"
#include "DX11ResourceBuffer.h"

#include "Device/Geometry/IndexBuffer.h"
#include "Device/Geometry/VertexBuffer.h"
#include "Device/PresentationParameters.h"

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
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceEncapsulator::DeviceEncapsulator(Graphics::DeviceEncapsulator *owner, void *windowHandle, const PresentationParameters& pp)
:   AbstractDeviceAPIEncapsulator(DeviceAPI::DirectX11, owner, pp) {
    _wrapper.Create(this, windowHandle, Parameters());
    _writer = new ConstantWriter(this);

    ChangeStatus(DeviceStatus::Normal, DeviceStatus::Invalid);
}
//----------------------------------------------------------------------------
DeviceEncapsulator::~DeviceEncapsulator() {
    ChangeStatus(DeviceStatus::Destroy, DeviceStatus::Normal);

    _writer = nullptr;
    _wrapper.Destroy(this);

    ChangeStatus(DeviceStatus::Invalid, DeviceStatus::Destroy);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Reset(const PresentationParameters& pp) {
    ChangeStatus(DeviceStatus::Reset, DeviceStatus::Normal);
    // TODO
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Present() {
    const UINT syncInterval = UINT(Parameters().PresentationInterval());
    _wrapper.SwapChain()->Present(syncInterval, 0);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ClearState() {
    _wrapper.ImmediateContext()->ClearState();
    _wrapper.ImmediateContext()->Flush();

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Alpha/Raster/Depth State
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetViewport(const ViewportF& viewport) {
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
void DeviceEncapsulator::SetViewports(const MemoryView<const ViewportF>& viewports) {
    const UINT dx11NumViews = checked_cast<UINT>(viewports.size());
    const auto dx11Viewports = MALLOCA_VIEW(::D3D11_VIEWPORT, viewports.size());

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
DeviceAPIDependantBlendState *DeviceEncapsulator::CreateBlendState(Graphics::BlendState *state) {
    return new DX11::BlendState(this, state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetBlendState(const Graphics::BlendState *state) {
    ::ID3D11BlendState *const dx11BlendState =
        checked_cast<const DX11::BlendState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->OMSetBlendState(
        dx11BlendState,
        state->BlendFactor()._data,
        state->MultiSampleMask()
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyBlendState(Graphics::BlendState *state, PDeviceAPIDependantBlendState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantRasterizerState *DeviceEncapsulator::CreateRasterizerState(Graphics::RasterizerState *state) {
    return new DX11::RasterizerState(this, state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRasterizerState(const Graphics::RasterizerState *state) {
    ::ID3D11RasterizerState *const dx11RasterizerState =
        checked_cast<const DX11::RasterizerState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->RSSetState(dx11RasterizerState);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyRasterizerState(Graphics::RasterizerState *state, PDeviceAPIDependantRasterizerState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencilState *DeviceEncapsulator::CreateDepthStencilState(Graphics::DepthStencilState *state) {
    return new DX11::DepthStencilState(this, state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetDepthStencilState(const Graphics::DepthStencilState *state) {
    ::ID3D11DepthStencilState *const dx11DepthStencilState =
        checked_cast<const DX11::DepthStencilState *>(state->DeviceAPIDependantState().get())->Entity();

    _wrapper.ImmediateContext()->OMSetDepthStencilState(
        dx11DepthStencilState,
        state->RendererStencil()
        );

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyDepthStencilState(Graphics::DepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantSamplerState *DeviceEncapsulator::CreateSamplerState(Graphics::SamplerState *state) {
    return new DX11::SamplerState(this, state);
}
//---------------------------------------------------------------------------
void DeviceEncapsulator::SetSamplerState(ShaderProgramType stage, size_t slot, const Graphics::SamplerState *state) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11SamplerState *const dx11SamplerState =
        checked_cast<const DX11::SamplerState *>(state->DeviceAPIDependantState().get())->Entity();

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        context->VSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case Core::Graphics::ShaderProgramType::Hull:
        context->HSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case Core::Graphics::ShaderProgramType::Domain:
        context->DSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case Core::Graphics::ShaderProgramType::Pixel:
        context->PSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case Core::Graphics::ShaderProgramType::Geometry:
        context->GSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    case Core::Graphics::ShaderProgramType::Compute:
        context->CSSetSamplers(dx11Slot, 1, &dx11SamplerState);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//---------------------------------------------------------------------------
void DeviceEncapsulator::DestroySamplerState(Graphics::SamplerState *state, PDeviceAPIDependantSamplerState& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Index/Vertex Buffer
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration *DeviceEncapsulator::CreateVertexDeclaration(Graphics::VertexDeclaration *declaration) {
    return new DX11::VertexDeclaration(this, declaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyVertexDeclaration(Graphics::VertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateIndexBuffer(Graphics::IndexBuffer *indexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    return new DX11::ResourceBuffer(this, resourceBuffer, indexBuffer, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetIndexBuffer(const Graphics::IndexBuffer *indexBuffer) {
    SetIndexBuffer(indexBuffer, 0);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetIndexBuffer(const Graphics::IndexBuffer *indexBuffer, size_t offset) {
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
void DeviceEncapsulator::DestroyIndexBuffer(Graphics::IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateVertexBuffer(Graphics::VertexBuffer *vertexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    return new DX11::ResourceBuffer(this, resourceBuffer, vertexBuffer, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const Graphics::VertexBuffer *vertexBuffer) {
    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(vertexBuffer->Buffer());

    const UINT stride = checked_cast<UINT>(vertexBuffer->VertexDeclaration()->SizeInBytes());
    const UINT offset = 0;

    _wrapper.ImmediateContext()->IASetVertexBuffers(0, 1, &dx11Buffer, &stride, &offset);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const Graphics::VertexBuffer *vertexBuffer, u32 vertexOffset) {
    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(vertexBuffer->Buffer());

    const UINT stride = checked_cast<UINT>(vertexBuffer->VertexDeclaration()->SizeInBytes());
    const UINT offset = checked_cast<UINT>(vertexOffset);

    _wrapper.ImmediateContext()->IASetVertexBuffers(0, 1, &dx11Buffer, &stride, &offset);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const MemoryView<const VertexBufferBinding>& bindings) {
    const size_t k = bindings.size();

    const auto dx11Buffers = MALLOCA_VIEW(::ID3D11Buffer *, k);
    const auto strideAndOffset = MALLOCA_VIEW(UINT, k*2);

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
void DeviceEncapsulator::DestroyVertexBuffer(Graphics::VertexBuffer *buffer, PDeviceAPIDependantResourceBuffer& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Shaders
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateConstantBuffer(Graphics::ConstantBuffer *constantBuffer, DeviceResourceBuffer *resourceBuffer, PDeviceAPIDependantConstantWriter& writer) {
    writer = _writer;
    return new DX11::ResourceBuffer(this, resourceBuffer, constantBuffer, MemoryView<const u8>());
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetConstantBuffer(ShaderProgramType stage, size_t slot, const Graphics::ConstantBuffer *constantBuffer) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11Buffer *const dx11Buffer = DX11DeviceBufferEntity(constantBuffer->Buffer());

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        context->VSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case Core::Graphics::ShaderProgramType::Hull:
        context->HSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case Core::Graphics::ShaderProgramType::Domain:
        context->DSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case Core::Graphics::ShaderProgramType::Pixel:
        context->PSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case Core::Graphics::ShaderProgramType::Geometry:
        context->GSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    case Core::Graphics::ShaderProgramType::Compute:
        context->CSSetConstantBuffers(dx11Slot, 1, &dx11Buffer);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyConstantBuffer(Graphics::ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity, PDeviceAPIDependantConstantWriter& writer) {
    Assert(writer = _writer);
    writer = nullptr;
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderProgram *DeviceEncapsulator::CreateShaderProgram(
    Graphics::ShaderProgram *program,
    const char *entryPoint,
    Graphics::ShaderCompilerFlags flags,
    const Graphics::ShaderSource *source,
    const Graphics::VertexDeclaration *vertexDeclaration) {
    return new DX11::ShaderProgram(this, program,  entryPoint, flags, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyShaderProgram(Graphics::ShaderProgram *program, PDeviceAPIDependantShaderProgram& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::PreprocessShaderProgram(
    RAWSTORAGE(Shader, char)& output,
    const Graphics::ShaderProgram *program,
    const Graphics::ShaderSource *source,
    const Graphics::VertexDeclaration *vertexDeclaration) {
    DX11::ShaderProgram::Preprocess(this, output, program, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ReflectShaderProgram(
    ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
    VECTOR(Shader, ShaderProgramTexture)& textures,
    const Graphics::ShaderProgram *program) {
    DX11::ShaderProgram::Reflect(this, constants, textures, program);
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect *DeviceEncapsulator::CreateShaderEffect(Graphics::ShaderEffect *effect) {
    return new DX11::ShaderEffect(this, effect);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetShaderEffect(const Graphics::ShaderEffect *effect) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    const DX11::ShaderEffect *dx11Effect = checked_cast<DX11::ShaderEffect *>(effect->DeviceAPIDependantEffect().get());

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
void DeviceEncapsulator::DestroyShaderEffect(Graphics::ShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
// Textures
//---------------------------------------------------------------------------
DeviceAPIDependantTexture2D *DeviceEncapsulator::CreateTexture2D(Graphics::Texture2D *texture, const MemoryView<const u8>& optionalData) {
    return new DX11::Texture2D(this, texture, optionalData);
}
//---------------------------------------------------------------------------
void DeviceEncapsulator::DestroyTexture2D(Graphics::Texture2D *texture, PDeviceAPIDependantTexture2D& entity) {
    RemoveRef_AssertReachZero(entity);
}
//---------------------------------------------------------------------------
DeviceAPIDependantTextureCube *DeviceEncapsulator::CreateTextureCube(Graphics::TextureCube *texture, const MemoryView<const u8>& optionalData) {
    return new DX11::TextureCube(this, texture, optionalData);
}
//---------------------------------------------------------------------------
void DeviceEncapsulator::DestroyTextureCube(Graphics::TextureCube *texture, PDeviceAPIDependantTextureCube& entity) {
    RemoveRef_AssertReachZero(entity);
}
//---------------------------------------------------------------------------
void DeviceEncapsulator::SetTexture(ShaderProgramType stage, size_t slot, const Graphics::Texture *texture) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11ShaderResourceView *dx11ResourceView = nullptr;

    if (texture) {
        Assert(texture->Frozen());
        Assert(texture->Available());

        const IDeviceAPIDependantAbstractTextureContent *content = texture->DeviceAPIDependantTexture()->Content();
        if (!content)
            throw DeviceEncapsulatorException("DX11: empty texture content could not be set to device", this, texture);

        const DX11::AbstractTextureContent *dx11Content = checked_cast<const DX11::AbstractTextureContent *>(content);

        dx11ResourceView = dx11Content->ShaderView();
        AssertRelease(dx11ResourceView);
    }

    const UINT dx11Slot = checked_cast<UINT>(slot);

    switch (stage)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        context->VSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case Core::Graphics::ShaderProgramType::Hull:
        context->HSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case Core::Graphics::ShaderProgramType::Domain:
        context->DSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case Core::Graphics::ShaderProgramType::Pixel:
        context->PSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case Core::Graphics::ShaderProgramType::Geometry:
        context->GSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    case Core::Graphics::ShaderProgramType::Compute:
        context->CSSetShaderResources(dx11Slot, 1, &dx11ResourceView);
        break;
    default:
        AssertNotImplemented();
    }

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Render Targets
//----------------------------------------------------------------------------
Graphics::RenderTarget *DeviceEncapsulator::BackBufferRenderTarget() {
    return _wrapper.BackBufferRenderTarget();
}
//----------------------------------------------------------------------------
Graphics::DepthStencil *DeviceEncapsulator::BackBufferDepthStencil() {
    return _wrapper.BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
const Graphics::RenderTarget *DeviceEncapsulator::BackBufferRenderTarget() const {
    return _wrapper.BackBufferRenderTarget();
}
//----------------------------------------------------------------------------
const Graphics::DepthStencil *DeviceEncapsulator::BackBufferDepthStencil() const {
    return _wrapper.BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
DeviceAPIDependantRenderTarget *DeviceEncapsulator::CreateRenderTarget(Graphics::RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) {
    return new DX11::RenderTarget(this, renderTarget, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRenderTarget(const Graphics::RenderTarget *renderTarget, const Graphics::DepthStencil *depthStencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11RenderTargetView *dx11RenderTarget = nullptr;
    if (renderTarget) {
        dx11RenderTarget = checked_cast<const DX11::RenderTarget *>(renderTarget->DeviceAPIDependantTexture2D().get())->RenderTargetView();
        Assert(dx11RenderTarget);
    }

    ::ID3D11DepthStencilView *dx11DepthStencil = nullptr;
    if (depthStencil) {
        dx11DepthStencil = checked_cast<const DX11::DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
        Assert(dx11DepthStencil);
    }

    context->OMSetRenderTargets(1, &dx11RenderTarget, dx11DepthStencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const Graphics::DepthStencil *depthStencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    const auto dx11RenderTargets = MALLOCA_VIEW(::ID3D11RenderTargetView *, bindings.size());
    ::SecureZeroMemory(dx11RenderTargets.Pointer(), dx11RenderTargets.SizeInBytes());

    for (const RenderTargetBinding& binding : bindings) {
        Assert(!dx11RenderTargets[binding.Slot]);
        dx11RenderTargets[binding.Slot] = checked_cast<const DX11::RenderTarget *>(binding.RT->DeviceAPIDependantTexture2D().get())->RenderTargetView();
    }

    ::ID3D11DepthStencilView *dx11DepthStencil = nullptr;
    if (depthStencil) {
        dx11DepthStencil = checked_cast<const DX11::DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
        Assert(dx11DepthStencil);
    }

    UINT numViews = checked_cast<UINT>(bindings.size());

    context->OMSetRenderTargets(numViews, dx11RenderTargets.Pointer(), dx11DepthStencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyRenderTarget(Graphics::RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencil *DeviceEncapsulator::CreateDepthStencil(Graphics::DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) {
    return new DX11::DepthStencil(this, depthStencil, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyDepthStencil(Graphics::DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) {
    RemoveRef_AssertReachZero(entity);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Clear(const Graphics::RenderTarget *renderTarget, const ColorRGBAF& color) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11RenderTargetView *dx11RenderTarget =
        checked_cast<const DX11::RenderTarget *>(renderTarget->DeviceAPIDependantTexture2D().get())->RenderTargetView();
    Assert(dx11RenderTarget);

    const FLOAT dx11ColorRGBA[4] = {color.r(), color.g(), color.b(), color.a()};

    context->ClearRenderTargetView(dx11RenderTarget, dx11ColorRGBA);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Clear(const Graphics::DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();

    ::ID3D11DepthStencilView *dx11DepthStencil =
        checked_cast<const DX11::DepthStencil *>(depthStencil->DeviceAPIDependantTexture2D().get())->DepthStencilView();
    Assert(dx11DepthStencil);

    UINT dx11ClearFlags = 0;
    if (size_t(opts) & size_t(ClearOptions::Depth) )
        dx11ClearFlags |= D3D11_CLEAR_DEPTH;
    if (size_t(opts) & size_t(ClearOptions::Stencil) )
        dx11ClearFlags |= D3D11_CLEAR_STENCIL;

    context->ClearDepthStencilView(dx11DepthStencil, dx11ClearFlags, depth, stencil);

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
// Draw
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawPrimitives(Graphics::PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) {
    ::ID3D11DeviceContext *const context = _wrapper.ImmediateContext();
    ::D3D11_PRIMITIVE_TOPOLOGY dx11PrimitiveTopology = PrimitiveTypeToDX11PrimitiveTopology(primitiveType);

    context->IASetPrimitiveTopology(dx11PrimitiveTopology);
    context->Draw(
        checked_cast<UINT>(IndexCount(primitiveType, primitiveCount)),
        checked_cast<UINT>(startVertex));

    CHECK_DIRECTX11_ERROR();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawIndexedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) {
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
void DeviceEncapsulator::DrawInstancedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) {
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
bool DeviceEncapsulator::IsProfilerAttached() const {
#ifdef WITH_DIRECTX11_DEBUG_MARKERS
    return  nullptr != _wrapper.UserDefinedAnnotation()/* &&
            _wrapper.UserDefinedAnnotation()->GetStatus() */;
#else
    return false;
#endif
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::BeginEvent(const wchar_t *name) {
    Assert(_wrapper.UserDefinedAnnotation());

    _wrapper.UserDefinedAnnotation()->BeginEvent(name);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::EndEvent() {
    Assert(_wrapper.UserDefinedAnnotation());

    _wrapper.UserDefinedAnnotation()->EndEvent();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetMarker(const wchar_t *name) {
    Assert(_wrapper.UserDefinedAnnotation());

    _wrapper.UserDefinedAnnotation()->SetMarker(name);
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
