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
#include "Core/Memory/ComPtr.h"

#ifdef OS_WINDOWS
#   include "DirectX11/DX11DeviceAPIEncapsulator.h"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceEncapsulator::DeviceEncapsulator() {}
//----------------------------------------------------------------------------
DeviceEncapsulator::~DeviceEncapsulator() {
    AssertRelease(!_deviceAPIEncapsulator);
}
//----------------------------------------------------------------------------
DeviceAPI DeviceEncapsulator::API() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);
    return _deviceAPIEncapsulator->API();
}
//----------------------------------------------------------------------------
const PresentationParameters& DeviceEncapsulator::Parameters() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);
    return _deviceAPIEncapsulator->Parameters();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Create(DeviceAPI api, void *windowHandle, const PresentationParameters& presentationParameters) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_deviceAPIEncapsulator);

    LOG(Information, L"[DeviceEncapsulator] CreateDevice({0})", DeviceAPIToCStr(api));

    switch (api)
    {
    case Core::Graphics::DeviceAPI::DirectX11:
        _deviceAPIEncapsulator.reset(new DX11DeviceAPIEncapsulator(this, windowHandle, presentationParameters));
        break;

    case Core::Graphics::DeviceAPI::OpenGL4:
    default:
        AssertNotImplemented();
    }

    Assert(_deviceAPIEncapsulator);

    GraphicsStartup::OnDeviceCreate(this);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);

    LOG(Information, L"[DeviceEncapsulator] DestroyDevice({0})", DeviceAPIToCStr(_deviceAPIEncapsulator->API()));

     _deviceAPIEncapsulator->ClearState();

    GraphicsStartup::OnDeviceDestroy(this);

    _deviceAPIEncapsulator.reset();

    Assert(!_deviceAPIEncapsulator);
}
//----------------------------------------------------------------------------
IDeviceAPIEncapsulator *DeviceEncapsulator::Device() const { 
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
IDeviceAPIContext *DeviceEncapsulator::Immediate() const { 
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
IDeviceAPIShaderCompiler *DeviceEncapsulator::ShaderCompiler() const { 
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
IDeviceAPIDiagnostics *DeviceEncapsulator::Diagnostics() const { 
    return const_cast<DeviceEncapsulator *>(this);
}
#endif
//----------------------------------------------------------------------------
// Status
//----------------------------------------------------------------------------
void DeviceEncapsulator::Reset(const PresentationParameters& pp) {
    THIS_THREADRESOURCE_CHECKACCESS();

    ++_revision.Value;

    _deviceAPIEncapsulator->Reset(pp);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Present() {
    THIS_THREADRESOURCE_CHECKACCESS();

    ++_revision.Value;

    _deviceAPIEncapsulator->Present();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ClearState() {
    THIS_THREADRESOURCE_CHECKACCESS();

    ++_revision.Value;

    _deviceAPIEncapsulator->ClearState();
}
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
// Alpha/Raster/Depth State
//----------------------------------------------------------------------------
DeviceAPIDependantBlendState *DeviceEncapsulator::CreateBlendState(BlendState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateBlendState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetBlendState(const BlendState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    _deviceAPIEncapsulator->Immediate()->SetBlendState(state);
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
DeviceAPIDependantRasterizerState *DeviceEncapsulator::CreateRasterizerState(RasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateRasterizerState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRasterizerState(const RasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    _deviceAPIEncapsulator->Immediate()->SetRasterizerState(state);
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
DeviceAPIDependantDepthStencilState *DeviceEncapsulator::CreateDepthStencilState(DepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateDepthStencilState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetDepthStencilState(const DepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    _deviceAPIEncapsulator->Immediate()->SetDepthStencilState(state);
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
DeviceAPIDependantSamplerState *DeviceEncapsulator::CreateSamplerState(SamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateSamplerState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetSamplerState(ShaderProgramType stage, size_t slot, const SamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    _deviceAPIEncapsulator->Immediate()->SetSamplerState(stage, slot, state);
}

//----------------------------------------------------------------------------
void DeviceEncapsulator::SetSamplerStates(ShaderProgramType stage, const MemoryView<const SamplerState *>& states) {
    THIS_THREADRESOURCE_CHECKACCESS();
#ifdef WITH_CORE_ASSERT
    for (const SamplerState *state : states) {
        Assert(state);
        Assert(state->Frozen());
        Assert(state->Available());
    }
#endif

    _deviceAPIEncapsulator->Immediate()->SetSamplerStates(stage, states);
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
// Index/Vertex Buffer
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
void DeviceEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(indexBuffer->Available());

    _deviceAPIEncapsulator->Immediate()->SetIndexBuffer(indexBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(indexBuffer->Available());
    Assert(offset < indexBuffer->IndexCount());

    _deviceAPIEncapsulator->Immediate()->SetIndexBuffer(indexBuffer, offset);
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
void DeviceEncapsulator::SetVertexBuffer(const VertexBuffer *vertexBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(vertexBuffer->Available());

    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(vertexBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const VertexBuffer *vertexBuffer, u32 vertexOffset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(vertexBuffer->Available());
    Assert(vertexOffset < vertexBuffer->VertexCount());

    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(vertexBuffer, vertexOffset);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const MemoryView<const VertexBufferBinding>& bindings) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(bindings.size());

#ifdef WITH_CORE_ASSERT
    for (const VertexBufferBinding& b : bindings) {
        Assert(b.VertexBuffer);
        Assert(b.VertexBuffer->Frozen());
        Assert(b.VertexBuffer->Available());
        Assert(b.VertexOffset < b.VertexBuffer->VertexCount());
    }
#endif

    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(bindings);
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
// Shaders
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
void DeviceEncapsulator::SetConstantBuffer(ShaderProgramType stage, size_t slot, const ConstantBuffer *constantBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(constantBuffer->Available());

    _deviceAPIEncapsulator->Immediate()->SetConstantBuffer(stage, slot, constantBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetConstantBuffers(ShaderProgramType stage, const MemoryView<const ConstantBuffer *>& constantBuffers) {
    THIS_THREADRESOURCE_CHECKACCESS();
#ifdef WITH_CORE_ASSERT
    for (const ConstantBuffer *constantBuffer : constantBuffers) {
        Assert(constantBuffer);
        Assert(constantBuffer->Frozen());
        Assert(constantBuffer->Available());
    }
#endif

    _deviceAPIEncapsulator->Immediate()->SetConstantBuffers(stage, constantBuffers);
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
DeviceAPIDependantShaderProgram *DeviceEncapsulator::CreateShaderProgram(
    ShaderProgram *program,
    const char *entryPoint,
    ShaderCompilerFlags flags,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(entryPoint);
    Assert(source);
    Assert(vertexDeclaration);

    return _deviceAPIEncapsulator->ShaderCompiler()->CreateShaderProgram(program, entryPoint, flags, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::PreprocessShaderProgram(
    RAWSTORAGE(Shader, char)& output,
    const ShaderProgram *program,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(source);
    Assert(vertexDeclaration);

    return _deviceAPIEncapsulator->ShaderCompiler()->PreprocessShaderProgram(output, program, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ReflectShaderProgram(
    ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
    VECTOR(Shader, ShaderProgramTexture)& textures,
    const ShaderProgram *program) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(program->Available());
    Assert(constants.empty());
    Assert(textures.empty());

    return _deviceAPIEncapsulator->ShaderCompiler()->ReflectShaderProgram(constants, textures, program);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyShaderProgram(ShaderProgram *program, PDeviceAPIDependantShaderProgram& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(entity);
    Assert(&entity == &program->DeviceAPIDependantProgram());

    _deviceAPIEncapsulator->ShaderCompiler()->DestroyShaderProgram(program, entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect *DeviceEncapsulator::CreateShaderEffect(ShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateShaderEffect(effect);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetShaderEffect(const ShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());
    Assert(effect->Available());

    _deviceAPIEncapsulator->Immediate()->SetShaderEffect(effect);
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
// Textures
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
void DeviceEncapsulator::SetTexture(ShaderProgramType stage, size_t slot, const Texture *texture) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!texture || texture->Frozen());
    Assert(!texture || texture->Available());

    _deviceAPIEncapsulator->Immediate()->SetTexture(stage, slot, texture);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetTextures(ShaderProgramType stage, const MemoryView<const Texture *>& textures) {
    THIS_THREADRESOURCE_CHECKACCESS();
#ifdef WITH_CORE_ASSERT
    for (const Texture *texture : textures) {
        if (!texture) continue;
        Assert(texture->Frozen());
        Assert(texture->Available());
    }
#endif

    _deviceAPIEncapsulator->Immediate()->SetTextures(stage, textures);
}
//----------------------------------------------------------------------------
// Render Targets
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
DeviceAPIDependantRenderTarget *DeviceEncapsulator::CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());

    return _deviceAPIEncapsulator->Device()->CreateRenderTarget(renderTarget, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRenderTarget(const RenderTarget *renderTarget, const DepthStencil *depthStencil) {
    Assert(!renderTarget || (renderTarget->Frozen() && renderTarget->Available()));
    Assert(!depthStencil || (depthStencil->Frozen() && depthStencil->Available()));

    _deviceAPIEncapsulator->Device()->SetRenderTarget(renderTarget, depthStencil);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const DepthStencil *depthStencil) {
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
void DeviceEncapsulator::DestroyRenderTarget(RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());
    Assert(entity);

    _deviceAPIEncapsulator->Device()->DestroyRenderTarget(renderTarget, entity);
}
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
// Draw
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawPrimitives(PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawPrimitives(primitiveType, startVertex, primitiveCount);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawIndexedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawIndexedPrimitives(primitiveType, baseVertex, startIndex, primitiveCount);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawInstancedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawInstancedPrimitives(primitiveType, baseVertex, startIndex, primitiveCount, startInstance, instanceCount);
}
//----------------------------------------------------------------------------
// Diagnostics
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
bool DeviceEncapsulator::IsProfilerAttached() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Diagnostics()->IsProfilerAttached();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::BeginEvent(const wchar_t *name) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    Assert(IsProfilerAttached());

    return _deviceAPIEncapsulator->Diagnostics()->BeginEvent(name);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::EndEvent() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsProfilerAttached());

    return _deviceAPIEncapsulator->Diagnostics()->EndEvent();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetMarker(const wchar_t *name) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    Assert(IsProfilerAttached());

    return _deviceAPIEncapsulator->Diagnostics()->SetMarker(name);
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *DeviceStatusToCStr(DeviceStatus status) {
    switch (status)
    {
    case Core::Graphics::DeviceStatus::Invalid:
        return "Invalid";
    case Core::Graphics::DeviceStatus::Normal:
        return "Normal";
    case Core::Graphics::DeviceStatus::Reset:
        return "Reset";
    case Core::Graphics::DeviceStatus::Lost:
        return "Lost";
    case Core::Graphics::DeviceStatus::Destroy:
        return "Destroy";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
