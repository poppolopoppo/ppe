#include "stdafx.h"

#include "DeviceEncapsulator.h"

#include "Geometry/IndexBuffer.h"
#include "Geometry/PrimitiveType.h"
#include "Geometry/VertexBuffer.h"
#include "Geometry/VertexDeclaration.h"

#include "Shader/ConstantBuffer.h"
#include "Shader/ShaderEffect.h"
#include "Shader/ShaderProgram.h"

#include "State/BlendState.h"
#include "State/DepthStencilState.h"
#include "State/RasterizerState.h"
#include "State/SamplerState.h"

#include "Texture/DepthStencil.h"
#include "Texture/RenderTarget.h"
#include "Texture/Texture.h"
#include "Texture/Texture2D.h"
#include "Texture/TextureCube.h"

#include "Core/Diagnostic/Logger.h"

// IDeviceAPIEncapsulator

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Viewport
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetViewport(const ViewportF& viewport) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(viewport.HasPositiveExtentsStrict());

    _deviceAPIEncapsulator->Device()->SetViewport(viewport);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetViewports(const MemoryView<const ViewportF>& viewports) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(viewports.size());

#ifdef WITH_CORE_ASSERT
    for (const ViewportF& viewport : viewports)
        Assert(viewport.HasPositiveExtentsStrict());
#endif

    _deviceAPIEncapsulator->Device()->SetViewports(viewports);
}
//----------------------------------------------------------------------------
// BlendState
//----------------------------------------------------------------------------
DeviceAPIDependantBlendState *DeviceEncapsulator::CreateBlendState(BlendState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateBlendState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyBlendState(BlendState *state, PDeviceAPIDependantBlendState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIEncapsulator->Device()->DestroyBlendState(state, entity);
}
//----------------------------------------------------------------------------
// RasterizerState
//----------------------------------------------------------------------------
DeviceAPIDependantRasterizerState *DeviceEncapsulator::CreateRasterizerState(RasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateRasterizerState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyRasterizerState(RasterizerState *state, PDeviceAPIDependantRasterizerState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIEncapsulator->Device()->DestroyRasterizerState(state, entity);
}
//----------------------------------------------------------------------------
// DepthStencilState
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencilState *DeviceEncapsulator::CreateDepthStencilState(DepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateDepthStencilState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyDepthStencilState(DepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIEncapsulator->Device()->DestroyDepthStencilState(state, entity);
}
//----------------------------------------------------------------------------
// SamplerState
//----------------------------------------------------------------------------
DeviceAPIDependantSamplerState *DeviceEncapsulator::CreateSamplerState(SamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateSamplerState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroySamplerState(SamplerState *state, PDeviceAPIDependantSamplerState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIEncapsulator->Device()->DestroySamplerState(state, entity);
}
//----------------------------------------------------------------------------
// VertexDeclaration
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration *DeviceEncapsulator::CreateVertexDeclaration(VertexDeclaration *declaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(declaration);
    Assert(declaration->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateVertexDeclaration(declaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyVertexDeclaration(VertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(declaration);
    Assert(declaration->Frozen());
    Assert(entity);
    Assert(&entity == &declaration->DeviceAPIDependantDeclaration());

    _deviceAPIEncapsulator->Device()->DestroyVertexDeclaration(declaration, entity);
}
//----------------------------------------------------------------------------
// IndexBuffer
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateIndexBuffer(IndexBuffer *indexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&indexBuffer->Buffer() == resourceBuffer);
    Assert(resourceBuffer->Usage() != BufferUsage::Immutable || optionalData.SizeInBytes() == resourceBuffer->SizeInBytes());

    return _deviceAPIEncapsulator->Device()->CreateIndexBuffer(indexBuffer, resourceBuffer, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyIndexBuffer(IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &indexBuffer->Buffer().DeviceAPIDependantBuffer());

    _deviceAPIEncapsulator->Device()->DestroyIndexBuffer(indexBuffer, entity);
}
//----------------------------------------------------------------------------
// VertexBuffer
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateVertexBuffer(VertexBuffer *vertexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&vertexBuffer->Buffer() == resourceBuffer);
    Assert(resourceBuffer->Usage() != BufferUsage::Immutable || optionalData.SizeInBytes() == resourceBuffer->SizeInBytes());

    return _deviceAPIEncapsulator->Device()->CreateVertexBuffer(vertexBuffer, resourceBuffer, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyVertexBuffer(VertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &vertexBuffer->Buffer().DeviceAPIDependantBuffer());

    _deviceAPIEncapsulator->Device()->DestroyVertexBuffer(vertexBuffer, entity);
}
//----------------------------------------------------------------------------
// ConstantBuffer
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateConstantBuffer(ConstantBuffer *constantBuffer, DeviceResourceBuffer *resourceBuffer, PDeviceAPIDependantConstantWriter& writer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&constantBuffer->Buffer() == resourceBuffer);
    Assert(!writer);

    DeviceAPIDependantResourceBuffer *const result =
        _deviceAPIEncapsulator->Device()->CreateConstantBuffer(constantBuffer, resourceBuffer, writer);

    Assert(writer);
    return result;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyConstantBuffer(ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity, PDeviceAPIDependantConstantWriter& writer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &constantBuffer->Buffer().DeviceAPIDependantBuffer());
    Assert(writer);

    _deviceAPIEncapsulator->Device()->DestroyConstantBuffer(constantBuffer, entity, writer);
}
//----------------------------------------------------------------------------
// ShaderEffect
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect *DeviceEncapsulator::CreateShaderEffect(ShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateShaderEffect(effect);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyShaderEffect(ShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());
    Assert(entity);
    Assert(&entity == &effect->DeviceAPIDependantEffect());

    _deviceAPIEncapsulator->Device()->DestroyShaderEffect(effect, entity);
}
//----------------------------------------------------------------------------
// Texture2D
//----------------------------------------------------------------------------
DeviceAPIDependantTexture2D *DeviceEncapsulator::CreateTexture2D(Texture2D *texture, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(texture->Usage() != BufferUsage::Immutable || optionalData.SizeInBytes() == texture->SizeInBytes());

    return _deviceAPIEncapsulator->Device()->CreateTexture2D(texture, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyTexture2D(Texture2D *texture, PDeviceAPIDependantTexture2D& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(entity);
    Assert(&entity == &texture->DeviceAPIDependantTexture2D());

    _deviceAPIEncapsulator->Device()->DestroyTexture2D(texture, entity);
}
//----------------------------------------------------------------------------
// TextureCube
//----------------------------------------------------------------------------
DeviceAPIDependantTextureCube *DeviceEncapsulator::CreateTextureCube(TextureCube *texture, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(texture->Usage() != BufferUsage::Immutable || optionalData.SizeInBytes() == texture->SizeInBytes());

    return _deviceAPIEncapsulator->Device()->CreateTextureCube(texture, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyTextureCube(TextureCube *texture, PDeviceAPIDependantTextureCube& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(entity);
    Assert(&entity == &texture->DeviceAPIDependantTextureCube());

    _deviceAPIEncapsulator->Device()->DestroyTextureCube(texture, entity);
}
//----------------------------------------------------------------------------
// BackBuffer
//----------------------------------------------------------------------------
RenderTarget *DeviceEncapsulator::BackBufferRenderTarget() {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->BackBufferRenderTarget();
}
//----------------------------------------------------------------------------
DepthStencil *DeviceEncapsulator::BackBufferDepthStencil() {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
const RenderTarget *DeviceEncapsulator::BackBufferRenderTarget() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->BackBufferRenderTarget();
}
//----------------------------------------------------------------------------
const DepthStencil *DeviceEncapsulator::BackBufferDepthStencil() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
// Render Target
//----------------------------------------------------------------------------
DeviceAPIDependantRenderTarget *DeviceEncapsulator::CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateRenderTarget(renderTarget, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyRenderTarget(RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());
    Assert(entity);

    _deviceAPIEncapsulator->Device()->DestroyRenderTarget(renderTarget, entity);
}
//----------------------------------------------------------------------------
// DepthStencil
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencil *DeviceEncapsulator::CreateDepthStencil(DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateDepthStencil(depthStencil, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyDepthStencil(DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());
    Assert(entity);

    _deviceAPIEncapsulator->Device()->DestroyDepthStencil(depthStencil, entity);
}
//----------------------------------------------------------------------------
// Clear
//----------------------------------------------------------------------------
void DeviceEncapsulator::Clear(const RenderTarget *renderTarget, const ColorRGBAF& color) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());
    Assert(renderTarget->Available());

    _deviceAPIEncapsulator->Device()->Clear(renderTarget, color);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Clear(const Graphics::DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());
    Assert(depthStencil->Available());

    _deviceAPIEncapsulator->Device()->Clear(depthStencil, opts, depth, stencil);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
