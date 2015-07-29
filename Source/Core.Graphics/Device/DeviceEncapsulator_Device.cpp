#include "stdafx.h"

#include "DeviceEncapsulator.h"

#include "AbstractDeviceAPIEncapsulator.h"

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

    DeviceAPIDependantBlendState *const entity = _deviceAPIEncapsulator->Device()->CreateBlendState(state);
    OnCreateEntity(state, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyBlendState(BlendState *state, PDeviceAPIDependantBlendState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    OnDestroyEntity(state, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyBlendState(state, entity);
}
//----------------------------------------------------------------------------
// RasterizerState
//----------------------------------------------------------------------------
DeviceAPIDependantRasterizerState *DeviceEncapsulator::CreateRasterizerState(RasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    DeviceAPIDependantRasterizerState *const entity = _deviceAPIEncapsulator->Device()->CreateRasterizerState(state);
    OnCreateEntity(state, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyRasterizerState(RasterizerState *state, PDeviceAPIDependantRasterizerState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    OnDestroyEntity(state, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyRasterizerState(state, entity);
}
//----------------------------------------------------------------------------
// DepthStencilState
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencilState *DeviceEncapsulator::CreateDepthStencilState(DepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    DeviceAPIDependantDepthStencilState *const entity = _deviceAPIEncapsulator->Device()->CreateDepthStencilState(state);
    OnCreateEntity(state, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyDepthStencilState(DepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    OnDestroyEntity(state, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyDepthStencilState(state, entity);
}
//----------------------------------------------------------------------------
// SamplerState
//----------------------------------------------------------------------------
DeviceAPIDependantSamplerState *DeviceEncapsulator::CreateSamplerState(SamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    DeviceAPIDependantSamplerState *const entity = _deviceAPIEncapsulator->Device()->CreateSamplerState(state);
    OnCreateEntity(state, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroySamplerState(SamplerState *state, PDeviceAPIDependantSamplerState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    OnDestroyEntity(state, entity.get());
    _deviceAPIEncapsulator->Device()->DestroySamplerState(state, entity);
}
//----------------------------------------------------------------------------
// VertexDeclaration
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration *DeviceEncapsulator::CreateVertexDeclaration(VertexDeclaration *declaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(declaration);
    Assert(declaration->Frozen());

    DeviceAPIDependantVertexDeclaration *const entity = _deviceAPIEncapsulator->Device()->CreateVertexDeclaration(declaration);
    OnCreateEntity(declaration, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyVertexDeclaration(VertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(declaration);
    Assert(declaration->Frozen());
    Assert(entity);
    Assert(&entity == &declaration->DeviceAPIDependantDeclaration());

    OnDestroyEntity(declaration, entity.get());
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

    DeviceAPIDependantResourceBuffer *const entity =
        _deviceAPIEncapsulator->Device()->CreateIndexBuffer(indexBuffer, resourceBuffer, optionalData);
    OnCreateEntity(indexBuffer, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyIndexBuffer(IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &indexBuffer->Buffer().DeviceAPIDependantBuffer());

    OnDestroyEntity(indexBuffer, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyIndexBuffer(indexBuffer, entity);
}
//---------------------------------------------------------------------------
// VertexBuffer
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateVertexBuffer(VertexBuffer *vertexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&vertexBuffer->Buffer() == resourceBuffer);
    Assert(resourceBuffer->Usage() != BufferUsage::Immutable || optionalData.SizeInBytes() == resourceBuffer->SizeInBytes());

    DeviceAPIDependantResourceBuffer *const entity =
        _deviceAPIEncapsulator->Device()->CreateVertexBuffer(vertexBuffer, resourceBuffer, optionalData);
    OnCreateEntity(vertexBuffer, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyVertexBuffer(VertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &vertexBuffer->Buffer().DeviceAPIDependantBuffer());

    OnDestroyEntity(vertexBuffer, entity.get());
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

    DeviceAPIDependantResourceBuffer *const entity =
        _deviceAPIEncapsulator->Device()->CreateConstantBuffer(constantBuffer, resourceBuffer, writer);
    OnCreateEntity(constantBuffer, entity);

    Assert(writer);
    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyConstantBuffer(ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity, PDeviceAPIDependantConstantWriter& writer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &constantBuffer->Buffer().DeviceAPIDependantBuffer());
    Assert(writer);

    OnDestroyEntity(constantBuffer, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyConstantBuffer(constantBuffer, entity, writer);
}
//----------------------------------------------------------------------------
// ShaderEffect
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect *DeviceEncapsulator::CreateShaderEffect(ShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());

    DeviceAPIDependantShaderEffect *const entity = _deviceAPIEncapsulator->Device()->CreateShaderEffect(effect);
    OnCreateEntity(effect, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyShaderEffect(ShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());
    Assert(entity);
    Assert(&entity == &effect->DeviceAPIDependantEffect());

    OnDestroyEntity(effect, entity.get());
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

    DeviceAPIDependantTexture2D *const entity =_deviceAPIEncapsulator->Device()->CreateTexture2D(texture, optionalData);
    OnCreateEntity(texture, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyTexture2D(Texture2D *texture, PDeviceAPIDependantTexture2D& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(entity);
    Assert(&entity == &texture->DeviceAPIDependantTexture2D());

    OnDestroyEntity(texture, entity.get());
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

    DeviceAPIDependantTextureCube *const entity = _deviceAPIEncapsulator->Device()->CreateTextureCube(texture, optionalData);
    OnCreateEntity(texture, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyTextureCube(TextureCube *texture, PDeviceAPIDependantTextureCube& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(entity);
    Assert(&entity == &texture->DeviceAPIDependantTextureCube());

    OnDestroyEntity(texture, entity.get());
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
void DeviceEncapsulator::SetRenderTarget(const RenderTarget *renderTarget, const DepthStencil *depthStencil) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!renderTarget || (renderTarget->Frozen() && renderTarget->Available()));
    Assert(!depthStencil || (depthStencil->Frozen() && depthStencil->Available()));

    _deviceAPIEncapsulator->Device()->SetRenderTarget(renderTarget, depthStencil);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const DepthStencil *depthStencil) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!depthStencil || (depthStencil->Frozen() && depthStencil->Available()));

#ifdef WITH_CORE_ASSERT
    Assert(bindings.size());
    for (const RenderTargetBinding& b : bindings) {
        Assert(b.RT);
        Assert(b.RT->Frozen());
        Assert(b.RT->Available());
        Assert(b.Slot < bindings.size());
    }
#endif

    _deviceAPIEncapsulator->Device()->SetRenderTargets(bindings, depthStencil);
}
//----------------------------------------------------------------------------
DeviceAPIDependantRenderTarget *DeviceEncapsulator::CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());

    DeviceAPIDependantRenderTarget *const entity = _deviceAPIEncapsulator->Device()->CreateRenderTarget(renderTarget, optionalData);
    OnCreateEntity(renderTarget, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyRenderTarget(RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());
    Assert(entity);

    OnDestroyEntity(renderTarget, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyRenderTarget(renderTarget, entity);
}
//----------------------------------------------------------------------------
// DepthStencil
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencil *DeviceEncapsulator::CreateDepthStencil(DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());

    DeviceAPIDependantDepthStencil *const entity = _deviceAPIEncapsulator->Device()->CreateDepthStencil(depthStencil, optionalData);
    OnCreateEntity(depthStencil, entity);

    return entity;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyDepthStencil(DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());
    Assert(entity);

    OnDestroyEntity(depthStencil, entity.get());
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
