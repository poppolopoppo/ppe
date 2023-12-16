// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DeviceEncapsulator.h"

#include "AbstractDeviceAPIEncapsulator.h"

#include "Geometry/IndexBuffer.h"
#include "Geometry/PrimitiveType.h"
#include "Geometry/VertexBuffer.h"
#include "Geometry/VertexDeclaration.h"

#include "Shader/ConstantBuffer.h"
#include "Shader/ShaderCompiled.h"
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

#include "Pool/DeviceSharedEntityKey.h"
#include "Pool/DeviceSharedEntityPool.h"
#include "Pool/DeviceResourceSharable.h"

#include "Diagnostic/Logger.h"

// IDeviceAPIEncapsulator

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Entity>
static bool TryPoolAllocate_Cooperative_(
    TRefPtr<const _Entity>& dst,
    const FDeviceResourceSharable& resource,
    FDeviceSharedEntityPool& pool ) {
    Assert(!dst);

    if (resource.Sharable() == false)
        return false;

    PCDeviceAPIDependantEntity entity;
    if (pool.Acquire_Cooperative(&entity, resource) == false)
        return false;

    dst.reset(checked_cast<const _Entity *>(entity.get()) );
    Assert(dst);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Entity>
static bool TryPoolDeallocate_Cooperative_(
    TRefPtr<const _Entity>& dst,
    const FDeviceResourceSharable& resource,
    FDeviceSharedEntityPool& pool ) {
    Assert(dst);

    if (resource.Sharable() == false)
        return false;

    PCDeviceAPIDependantEntity entity(dst.get());
    pool.Release_Cooperative(resource.SharedKey(), entity);

    RemoveRef_AssertGreaterThanZero(dst);
    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Entity>
static bool TryPoolAllocate_Exclusive_(
    TRefPtr<_Entity>& dst,
    const FDeviceResourceSharable& resource,
    FDeviceSharedEntityPool& pool ) {
    Assert(!dst);

    if (resource.Sharable() == false)
        return false;

    PDeviceAPIDependantEntity entity;
    if (pool.Acquire_Exclusive(&entity, resource) == false)
        return false;

    dst.reset(checked_cast<_Entity *>(entity.get()) );
    Assert(dst);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Entity>
static bool TryPoolDeallocate_Exclusive_(
    TRefPtr<_Entity>& dst,
    const FDeviceResourceSharable& resource,
    FDeviceSharedEntityPool& pool ) {
    Assert(dst);

    if (resource.Sharable() == false)
        return false;

    PDeviceAPIDependantEntity entity(dst.get());
    pool.Release_Exclusive(resource.SharedKey(), entity);

    RemoveRef_AssertGreaterThanZero(dst);
    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Viewport
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetViewport(const FViewport& viewport) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(viewport.HasPositiveExtentsStrict());

    _deviceAPIEncapsulator->Device()->SetViewport(viewport);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetViewports(const TMemoryView<const FViewport>& viewports) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(viewports.size());

#ifdef WITH_PPE_ASSERT
    for (const FViewport& viewport : viewports)
        Assert(viewport.HasPositiveExtentsStrict());
#endif

    _deviceAPIEncapsulator->Device()->SetViewports(viewports);
}
//----------------------------------------------------------------------------
// FBlendState
//----------------------------------------------------------------------------
FDeviceAPIDependantBlendState *FDeviceEncapsulator::CreateBlendState(FBlendState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    FDeviceAPIDependantBlendState *const entity = _deviceAPIEncapsulator->Device()->CreateBlendState(state);
    _deviceAPIEncapsulator->OnCreateEntity(state, entity);

    return entity;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyBlendState(FBlendState *state, PDeviceAPIDependantBlendState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIEncapsulator->OnDestroyEntity(state, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyBlendState(state, entity);
}
//----------------------------------------------------------------------------
// FRasterizerState
//----------------------------------------------------------------------------
FDeviceAPIDependantRasterizerState *FDeviceEncapsulator::CreateRasterizerState(FRasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    FDeviceAPIDependantRasterizerState *const entity = _deviceAPIEncapsulator->Device()->CreateRasterizerState(state);
    _deviceAPIEncapsulator->OnCreateEntity(state, entity);

    return entity;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyRasterizerState(FRasterizerState *state, PDeviceAPIDependantRasterizerState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIEncapsulator->OnDestroyEntity(state, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyRasterizerState(state, entity);
}
//----------------------------------------------------------------------------
// FDepthStencilState
//----------------------------------------------------------------------------
FDeviceAPIDependantDepthStencilState *FDeviceEncapsulator::CreateDepthStencilState(FDepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    FDeviceAPIDependantDepthStencilState *const entity = _deviceAPIEncapsulator->Device()->CreateDepthStencilState(state);
    _deviceAPIEncapsulator->OnCreateEntity(state, entity);

    return entity;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyDepthStencilState(FDepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIEncapsulator->OnDestroyEntity(state, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyDepthStencilState(state, entity);
}
//----------------------------------------------------------------------------
// FSamplerState
//----------------------------------------------------------------------------
FDeviceAPIDependantSamplerState *FDeviceEncapsulator::CreateSamplerState(FSamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    FDeviceAPIDependantSamplerState *const entity = _deviceAPIEncapsulator->Device()->CreateSamplerState(state);
    _deviceAPIEncapsulator->OnCreateEntity(state, entity);

    return entity;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroySamplerState(FSamplerState *state, PDeviceAPIDependantSamplerState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIEncapsulator->OnDestroyEntity(state, entity.get());
    _deviceAPIEncapsulator->Device()->DestroySamplerState(state, entity);
}
//----------------------------------------------------------------------------
// FVertexDeclaration
//----------------------------------------------------------------------------
FDeviceAPIDependantVertexDeclaration *FDeviceEncapsulator::CreateVertexDeclaration(FVertexDeclaration *declaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(declaration);
    Assert(declaration->Frozen());

    FDeviceAPIDependantVertexDeclaration *const entity = _deviceAPIEncapsulator->Device()->CreateVertexDeclaration(declaration);
    _deviceAPIEncapsulator->OnCreateEntity(declaration, entity);

    return entity;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyVertexDeclaration(FVertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(declaration);
    Assert(declaration->Frozen());
    Assert(entity);
    Assert(&entity == &declaration->DeviceAPIDependantDeclaration());

    _deviceAPIEncapsulator->OnDestroyEntity(declaration, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyVertexDeclaration(declaration, entity);
}
//----------------------------------------------------------------------------
// IndexBuffer
//----------------------------------------------------------------------------
FDeviceAPIDependantResourceBuffer *FDeviceEncapsulator::CreateIndexBuffer(FIndexBuffer *indexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&indexBuffer->Buffer() == resourceBuffer);
    Assert(resourceBuffer->Usage() != EBufferUsage::Immutable || optionalData.SizeInBytes() == resourceBuffer->SizeInBytes());

    PDeviceAPIDependantResourceBuffer entity;
    if (false == TryPoolAllocate_Exclusive_(entity, *indexBuffer, *_deviceSharedEntityPool))
        entity = _deviceAPIEncapsulator->Device()->CreateIndexBuffer(indexBuffer, resourceBuffer, optionalData);

    Assert(entity);
    _deviceAPIEncapsulator->OnCreateEntity(indexBuffer, entity.get());

    return RemoveRef_AssertReachZero_KeepAlive(entity);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyIndexBuffer(FIndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &indexBuffer->Buffer().DeviceAPIDependantBuffer());

    _deviceAPIEncapsulator->OnDestroyEntity(indexBuffer, entity.get());

    if (false == TryPoolDeallocate_Exclusive_(entity, *indexBuffer, *_deviceSharedEntityPool))
        _deviceAPIEncapsulator->Device()->DestroyIndexBuffer(indexBuffer, entity);
}
//---------------------------------------------------------------------------
// FVertexBuffer
//----------------------------------------------------------------------------
FDeviceAPIDependantResourceBuffer *FDeviceEncapsulator::CreateVertexBuffer(FVertexBuffer *vertexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&vertexBuffer->Buffer() == resourceBuffer);
    Assert(resourceBuffer->Usage() != EBufferUsage::Immutable || optionalData.SizeInBytes() == resourceBuffer->SizeInBytes());

    PDeviceAPIDependantResourceBuffer entity;
    if (false == TryPoolAllocate_Exclusive_(entity, *vertexBuffer, *_deviceSharedEntityPool))
        entity = _deviceAPIEncapsulator->Device()->CreateVertexBuffer(vertexBuffer, resourceBuffer, optionalData);

    Assert(entity);
    _deviceAPIEncapsulator->OnCreateEntity(vertexBuffer, entity.get());

    return RemoveRef_AssertReachZero_KeepAlive(entity);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyVertexBuffer(FVertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &vertexBuffer->Buffer().DeviceAPIDependantBuffer());

    _deviceAPIEncapsulator->OnDestroyEntity(vertexBuffer, entity.get());

    if (false == TryPoolDeallocate_Exclusive_(entity, *vertexBuffer, *_deviceSharedEntityPool))
        _deviceAPIEncapsulator->Device()->DestroyVertexBuffer(vertexBuffer, entity);
}
//----------------------------------------------------------------------------
// FConstantBuffer
//----------------------------------------------------------------------------
const FDeviceAPIDependantConstantWriter *FDeviceEncapsulator::ConstantWriter() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->ConstantWriter();
}
//----------------------------------------------------------------------------
FDeviceAPIDependantResourceBuffer *FDeviceEncapsulator::CreateConstantBuffer(FConstantBuffer *constantBuffer, FDeviceResourceBuffer *resourceBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&constantBuffer->Buffer() == resourceBuffer);

    PDeviceAPIDependantResourceBuffer entity;
    if (false == TryPoolAllocate_Exclusive_(entity, *constantBuffer, *_deviceSharedEntityPool))
        entity = _deviceAPIEncapsulator->Device()->CreateConstantBuffer(constantBuffer, resourceBuffer);

    Assert(entity);
    _deviceAPIEncapsulator->OnCreateEntity(constantBuffer, entity.get());

    return RemoveRef_AssertReachZero_KeepAlive(entity);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyConstantBuffer(FConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &constantBuffer->Buffer().DeviceAPIDependantBuffer());

    _deviceAPIEncapsulator->OnDestroyEntity(constantBuffer, entity.get());

    if (false == TryPoolDeallocate_Exclusive_(entity, *constantBuffer, *_deviceSharedEntityPool))
        _deviceAPIEncapsulator->Device()->DestroyConstantBuffer(constantBuffer, entity);
}
//----------------------------------------------------------------------------
// FShaderProgram
//----------------------------------------------------------------------------
FDeviceAPIDependantShaderProgram* FDeviceEncapsulator::CreateShaderProgram(FShaderProgram* program) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());

    PDeviceAPIDependantShaderProgram entity;
    if (false == TryPoolAllocate_Exclusive_(entity, *program, *_deviceSharedEntityPool))
        entity = _deviceAPIEncapsulator->Device()->CreateShaderProgram(program);

    Assert(entity);
    _deviceAPIEncapsulator->OnCreateEntity(program, entity.get());

    return RemoveRef_AssertReachZero_KeepAlive(entity);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyShaderProgram(FShaderProgram* program, PDeviceAPIDependantShaderProgram& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(entity);
    Assert(&entity == &program->DeviceAPIDependantProgram());

    _deviceAPIEncapsulator->OnDestroyEntity(program, entity.get());

    if (false == TryPoolDeallocate_Exclusive_(entity, *program, *_deviceSharedEntityPool))
        _deviceAPIEncapsulator->Device()->DestroyShaderProgram(program, entity);
}
//----------------------------------------------------------------------------
// FShaderEffect
//----------------------------------------------------------------------------
FDeviceAPIDependantShaderEffect *FDeviceEncapsulator::CreateShaderEffect(FShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());

    FDeviceAPIDependantShaderEffect *const entity = _deviceAPIEncapsulator->Device()->CreateShaderEffect(effect);
    _deviceAPIEncapsulator->OnCreateEntity(effect, entity);

    return entity;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyShaderEffect(FShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());
    Assert(entity);
    Assert(&entity == &effect->DeviceAPIDependantEffect());

    _deviceAPIEncapsulator->OnDestroyEntity(effect, entity.get());
    _deviceAPIEncapsulator->Device()->DestroyShaderEffect(effect, entity);
}
//----------------------------------------------------------------------------
// FTexture2D
//----------------------------------------------------------------------------
FDeviceAPIDependantTexture2D *FDeviceEncapsulator::CreateTexture2D(FTexture2D *texture, const TMemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(texture->Usage() != EBufferUsage::Immutable || optionalData.SizeInBytes() == texture->SizeInBytes());

    PDeviceAPIDependantTexture2D entity;
    if (false == TryPoolAllocate_Exclusive_(entity, *texture, *_deviceSharedEntityPool))
        entity = _deviceAPIEncapsulator->Device()->CreateTexture2D(texture, optionalData);

    Assert(entity);
    _deviceAPIEncapsulator->OnCreateEntity(texture, entity.get());

    return RemoveRef_AssertReachZero_KeepAlive(entity);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyTexture2D(FTexture2D *texture, PDeviceAPIDependantTexture2D& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(entity);
    Assert(&entity == &texture->DeviceAPIDependantTexture2D());

    _deviceAPIEncapsulator->OnDestroyEntity(texture, entity.get());

    if (false == TryPoolDeallocate_Exclusive_(entity, *texture, *_deviceSharedEntityPool))
        _deviceAPIEncapsulator->Device()->DestroyTexture2D(texture, entity);
}
//----------------------------------------------------------------------------
// FTextureCube
//----------------------------------------------------------------------------
FDeviceAPIDependantTextureCube *FDeviceEncapsulator::CreateTextureCube(FTextureCube *texture, const TMemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(texture->Usage() != EBufferUsage::Immutable || optionalData.SizeInBytes() == texture->SizeInBytes());

    PDeviceAPIDependantTextureCube entity;
    if (false == TryPoolAllocate_Exclusive_(entity, *texture, *_deviceSharedEntityPool))
        entity = _deviceAPIEncapsulator->Device()->CreateTextureCube(texture, optionalData);

    Assert(entity);
    _deviceAPIEncapsulator->OnCreateEntity(texture, entity.get());

    return RemoveRef_AssertReachZero_KeepAlive(entity);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyTextureCube(FTextureCube *texture, PDeviceAPIDependantTextureCube& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(entity);
    Assert(&entity == &texture->DeviceAPIDependantTextureCube());

    _deviceAPIEncapsulator->OnDestroyEntity(texture, entity.get());

    if (false == TryPoolDeallocate_Exclusive_(entity, *texture, *_deviceSharedEntityPool))
        _deviceAPIEncapsulator->Device()->DestroyTextureCube(texture, entity);
}
//----------------------------------------------------------------------------
// BackBuffer
//----------------------------------------------------------------------------
FRenderTarget *FDeviceEncapsulator::BackBufferRenderTarget() {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->BackBufferRenderTarget();
}
//----------------------------------------------------------------------------
FDepthStencil *FDeviceEncapsulator::BackBufferDepthStencil() {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
const FRenderTarget *FDeviceEncapsulator::BackBufferRenderTarget() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->BackBufferRenderTarget();
}
//----------------------------------------------------------------------------
const FDepthStencil *FDeviceEncapsulator::BackBufferDepthStencil() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Device()->BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
// Render Target
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetRenderTarget(const FRenderTarget *renderTarget, const FDepthStencil *depthStencil) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!renderTarget || (renderTarget->Frozen() && renderTarget->Available()));
    Assert(!depthStencil || (depthStencil->Frozen() && depthStencil->Available()));

    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), renderTarget ? renderTarget->ResourceName() : L"Null");

    _deviceAPIEncapsulator->Device()->SetRenderTarget(renderTarget, depthStencil);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetRenderTargets(const TMemoryView<const FRenderTargetBinding>& bindings, const FDepthStencil *depthStencil) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!depthStencil || (depthStencil->Frozen() && depthStencil->Available()));

#ifdef WITH_PPE_ASSERT
    Assert(bindings.size());
    for (const FRenderTargetBinding& b : bindings) {
        Assert(b.RT);
        Assert(b.RT->Frozen());
        Assert(b.RT->Available());
        Assert(b.Slot < bindings.size());
    }
#endif
#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
    for (const FRenderTargetBinding& b : bindings)
        GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), b.RT ? b.RT->ResourceName() : L"Null");
#endif

    _deviceAPIEncapsulator->Device()->SetRenderTargets(bindings, depthStencil);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantRenderTarget *FDeviceEncapsulator::CreateRenderTarget(FRenderTarget *renderTarget, const TMemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());

    PDeviceAPIDependantRenderTarget entity;
    if (false == TryPoolAllocate_Exclusive_(entity, *renderTarget, *_deviceSharedEntityPool))
        entity = _deviceAPIEncapsulator->Device()->CreateRenderTarget(renderTarget, optionalData);

    Assert(entity);
    _deviceAPIEncapsulator->OnCreateEntity(renderTarget, entity.get());

    return RemoveRef_AssertReachZero_KeepAlive(entity);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyRenderTarget(FRenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());
    Assert(entity);

    _deviceAPIEncapsulator->OnDestroyEntity(renderTarget, entity.get());

    if (false == TryPoolDeallocate_Exclusive_(entity, *renderTarget, *_deviceSharedEntityPool))
        _deviceAPIEncapsulator->Device()->DestroyRenderTarget(renderTarget, entity);
}
//----------------------------------------------------------------------------
// FDepthStencil
//----------------------------------------------------------------------------
FDeviceAPIDependantDepthStencil *FDeviceEncapsulator::CreateDepthStencil(FDepthStencil *depthStencil, const TMemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());

    PDeviceAPIDependantDepthStencil entity;
    if (false == TryPoolAllocate_Exclusive_(entity, *depthStencil, *_deviceSharedEntityPool))
        entity = _deviceAPIEncapsulator->Device()->CreateDepthStencil(depthStencil, optionalData);

    Assert(entity);
    _deviceAPIEncapsulator->OnCreateEntity(depthStencil, entity.get());

    return RemoveRef_AssertReachZero_KeepAlive(entity);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DestroyDepthStencil(FDepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());
    Assert(entity);

    _deviceAPIEncapsulator->OnDestroyEntity(depthStencil, entity.get());

    if (false == TryPoolDeallocate_Exclusive_(entity, *depthStencil, *_deviceSharedEntityPool))
        _deviceAPIEncapsulator->Device()->DestroyDepthStencil(depthStencil, entity);
}
//----------------------------------------------------------------------------
// Clear
//----------------------------------------------------------------------------
void FDeviceEncapsulator::Clear(const FRenderTarget *renderTarget, const FLinearColor& color) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());
    Assert(renderTarget->Available());

    _deviceAPIEncapsulator->Device()->Clear(renderTarget, color);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::Clear(const Graphics::FDepthStencil *depthStencil, EClearOptions opts, float depth, u8 stencil) {
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
} //!namespace PPE
