#include "stdafx.h"

#include "DeviceEncapsulator.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Memory/ComPtr.h"

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

#ifdef OS_WINDOWS
#   include "DirectX11/DX11DeviceEncapsulator.h"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AbstractDeviceAPIEncapsulator::AbstractDeviceAPIEncapsulator(
    DeviceAPI api,
    DeviceEncapsulator *owner,
    const PresentationParameters& pp)
:   _owner(owner)
,   _api(api)
,   _status(DeviceStatus::Invalid)
,   _parameters(pp) {
    Assert(owner);
}
//----------------------------------------------------------------------------
AbstractDeviceAPIEncapsulator::~AbstractDeviceAPIEncapsulator() {
    AssertRelease(_resources.empty());
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::RegisterResource(DeviceResource *resource) {
    Assert(resource);
    Assert(_resources.end() == std::find(_resources.begin(), _resources.end(), resource));

    _resources.push_back(resource);
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::UnregisterResource(DeviceResource *resource) {
    Assert(resource);

    const auto it = std::find(_resources.begin(), _resources.end(), resource);
    Assert(it != _resources.end());

    Erase_DontPreserveOrder(_resources, it);
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::ChangeStatus(DeviceStatus status, DeviceStatus old) {
    AssertRelease(old == _status);
    switch (_status = status)
    {
    case Core::Graphics::DeviceStatus::Invalid:
        Assert(DeviceStatus::Destroy == old);
        break;
    case Core::Graphics::DeviceStatus::Normal:
        Assert(DeviceStatus::Invalid == old);
        OnDeviceCreate();
        break;
    case Core::Graphics::DeviceStatus::Reset:
        Assert(DeviceStatus::Normal == old);
        OnDeviceReset();
        break;
    case Core::Graphics::DeviceStatus::Lost:
        Assert(DeviceStatus::Normal == old);
        OnDeviceLost();
        break;
    case Core::Graphics::DeviceStatus::Destroy:
        Assert(DeviceStatus::Normal == old);
        OnDeviceDestroy();
        break;
    default:
        AssertNotImplemented();
    };
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::ChangePresentationParameters(const PresentationParameters& parameters) {
    _parameters = parameters;
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::OnDeviceCreate() {
    Assert(DeviceStatus::Normal == _status);
    for (DeviceResource *resource : _resources)
        resource->OnDeviceCreate(_owner);
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::OnDeviceReset() {
    Assert(DeviceStatus::Reset == _status);
    for (DeviceResource *resource : _resources)
        resource->OnDeviceReset(_owner);
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::OnDeviceLost() {
    Assert(DeviceStatus::Lost == _status);
    for (DeviceResource *resource : _resources)
        resource->OnDeviceLost(_owner);
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::OnDeviceDestroy() {
    Assert(DeviceStatus::Destroy == _status);
    for (DeviceResource *resource : _resources)
        resource->OnDeviceDestroy(_owner);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceEncapsulator::DeviceEncapsulator() {}
//----------------------------------------------------------------------------
DeviceEncapsulator::~DeviceEncapsulator() {
    AssertRelease(!_deviceAPIDependantEncapsulator);
}
//----------------------------------------------------------------------------
DeviceAPI DeviceEncapsulator::API() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIDependantEncapsulator);
    return _deviceAPIDependantEncapsulator->API();
}
//----------------------------------------------------------------------------
DeviceStatus DeviceEncapsulator::Status() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIDependantEncapsulator);
    return _deviceAPIDependantEncapsulator->Status();
}
//----------------------------------------------------------------------------
const PresentationParameters& DeviceEncapsulator::Parameters() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIDependantEncapsulator);
    return _deviceAPIDependantEncapsulator->Parameters();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Create(DeviceAPI api, void *windowHandle, const PresentationParameters& presentationParameters) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_deviceAPIDependantEncapsulator);

    LOG(Information, L"[DeviceEncapsulator] CreateDevice({0})", DeviceAPIToCStr(api));

    switch (api)
    {
    case Core::Graphics::DeviceAPI::DirectX11:
        _deviceAPIDependantEncapsulator.reset(new DX11::DeviceEncapsulator(this, windowHandle, presentationParameters));
        break;

    case Core::Graphics::DeviceAPI::OpenGL4:
    default:
        AssertNotImplemented();
    }

    Assert(_deviceAPIDependantEncapsulator);

    GraphicsStartup::OnDeviceCreate(this);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIDependantEncapsulator);

    LOG(Information, L"[DeviceEncapsulator] DestroyDevice({0})", DeviceAPIToCStr(_deviceAPIDependantEncapsulator->API()));

     _deviceAPIDependantEncapsulator->ClearState();

    GraphicsStartup::OnDeviceDestroy(this);

    _deviceAPIDependantEncapsulator.reset();

    Assert(!_deviceAPIDependantEncapsulator);
}
//----------------------------------------------------------------------------
// Status
//----------------------------------------------------------------------------
void DeviceEncapsulator::Reset(const PresentationParameters& pp) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIDependantEncapsulator->Reset(pp);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Present() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIDependantEncapsulator->Present();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ClearState() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIDependantEncapsulator->ClearState();
}
//----------------------------------------------------------------------------
// Alpha/Raster/Depth State
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetViewport(const ViewportF& viewport) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(viewport.HasPositiveExtentsStrict());

    _deviceAPIDependantEncapsulator->Context()->SetViewport(viewport);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetViewports(const MemoryView<const ViewportF>& viewports) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(viewports.size());

#ifdef WITH_CORE_ASSERT
    for (const ViewportF& viewport : viewports)
        Assert(viewport.HasPositiveExtentsStrict());
#endif

    _deviceAPIDependantEncapsulator->Context()->SetViewports(viewports);
}
//----------------------------------------------------------------------------
DeviceAPIDependantBlendState *DeviceEncapsulator::CreateBlendState(BlendState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateBlendState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetBlendState(const BlendState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    _deviceAPIDependantEncapsulator->Context()->SetBlendState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyBlendState(BlendState *state, PDeviceAPIDependantBlendState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIDependantEncapsulator->Device()->DestroyBlendState(state, entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantRasterizerState *DeviceEncapsulator::CreateRasterizerState(RasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateRasterizerState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRasterizerState(const RasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    _deviceAPIDependantEncapsulator->Context()->SetRasterizerState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyRasterizerState(RasterizerState *state, PDeviceAPIDependantRasterizerState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIDependantEncapsulator->Device()->DestroyRasterizerState(state, entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencilState *DeviceEncapsulator::CreateDepthStencilState(DepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateDepthStencilState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetDepthStencilState(const DepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    _deviceAPIDependantEncapsulator->Context()->SetDepthStencilState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyDepthStencilState(DepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIDependantEncapsulator->Device()->DestroyDepthStencilState(state, entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantSamplerState *DeviceEncapsulator::CreateSamplerState(SamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateSamplerState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetSamplerState(ShaderProgramType stage, size_t slot, const SamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    _deviceAPIDependantEncapsulator->Context()->SetSamplerState(stage, slot, state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroySamplerState(SamplerState *state, PDeviceAPIDependantSamplerState& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(entity);
    Assert(&entity == &state->DeviceAPIDependantState());

    _deviceAPIDependantEncapsulator->Device()->DestroySamplerState(state, entity);
}
//----------------------------------------------------------------------------
// Index/Vertex Buffer
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration *DeviceEncapsulator::CreateVertexDeclaration(VertexDeclaration *declaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(declaration);
    Assert(declaration->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateVertexDeclaration(declaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyVertexDeclaration(VertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(declaration);
    Assert(declaration->Frozen());
    Assert(entity);
    Assert(&entity == &declaration->DeviceAPIDependantDeclaration());

    _deviceAPIDependantEncapsulator->Device()->DestroyVertexDeclaration(declaration, entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateIndexBuffer(IndexBuffer *indexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&indexBuffer->Buffer() == resourceBuffer);

    return _deviceAPIDependantEncapsulator->Device()->CreateIndexBuffer(indexBuffer, resourceBuffer, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(indexBuffer->Available());

    _deviceAPIDependantEncapsulator->Context()->SetIndexBuffer(indexBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(indexBuffer->Available());
    Assert(offset < indexBuffer->IndexCount());

    _deviceAPIDependantEncapsulator->Context()->SetIndexBuffer(indexBuffer, offset);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyIndexBuffer(IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &indexBuffer->Buffer().DeviceAPIDependantBuffer());

    _deviceAPIDependantEncapsulator->Device()->DestroyIndexBuffer(indexBuffer, entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer *DeviceEncapsulator::CreateVertexBuffer(VertexBuffer *vertexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(resourceBuffer);
    Assert(&vertexBuffer->Buffer() == resourceBuffer);

    return _deviceAPIDependantEncapsulator->Device()->CreateVertexBuffer(vertexBuffer, resourceBuffer, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const VertexBuffer *vertexBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(vertexBuffer->Available());

    _deviceAPIDependantEncapsulator->Context()->SetVertexBuffer(vertexBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const VertexBuffer *vertexBuffer, u32 vertexOffset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(vertexBuffer->Available());
    Assert(vertexOffset < vertexBuffer->VertexCount());

    _deviceAPIDependantEncapsulator->Context()->SetVertexBuffer(vertexBuffer, vertexOffset);
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

    _deviceAPIDependantEncapsulator->Context()->SetVertexBuffer(bindings);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyVertexBuffer(VertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &vertexBuffer->Buffer().DeviceAPIDependantBuffer());

    _deviceAPIDependantEncapsulator->Device()->DestroyVertexBuffer(vertexBuffer, entity);
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
        _deviceAPIDependantEncapsulator->Device()->CreateConstantBuffer(constantBuffer, resourceBuffer, writer);

    Assert(writer);
    return result;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetConstantBuffer(ShaderProgramType stage, size_t slot, const ConstantBuffer *constantBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(constantBuffer->Available());

    _deviceAPIDependantEncapsulator->Context()->SetConstantBuffer(stage, slot, constantBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyConstantBuffer(ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity, PDeviceAPIDependantConstantWriter& writer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(entity);
    //Assert(&entity == &constantBuffer->Buffer().DeviceAPIDependantBuffer());
    Assert(writer);

    _deviceAPIDependantEncapsulator->Device()->DestroyConstantBuffer(constantBuffer, entity, writer);
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

    return _deviceAPIDependantEncapsulator->Compiler()->CreateShaderProgram(program, entryPoint, flags, source, vertexDeclaration);
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

    return _deviceAPIDependantEncapsulator->Compiler()->PreprocessShaderProgram(output, program, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ReflectShaderProgram(
    ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
    VECTOR(Shader, BindName)& textures,
    const ShaderProgram *program) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(program->Available());
    Assert(constants.empty());
    Assert(textures.empty());

    return _deviceAPIDependantEncapsulator->Compiler()->ReflectShaderProgram(constants, textures, program);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyShaderProgram(ShaderProgram *program, PDeviceAPIDependantShaderProgram& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(entity);
    Assert(&entity == &program->DeviceAPIDependantProgram());

    _deviceAPIDependantEncapsulator->Compiler()->DestroyShaderProgram(program, entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect *DeviceEncapsulator::CreateShaderEffect(ShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateShaderEffect(effect);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetShaderEffect(const ShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());
    Assert(effect->Available());

    _deviceAPIDependantEncapsulator->Context()->SetShaderEffect(effect);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyShaderEffect(ShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());
    Assert(entity);
    Assert(&entity == &effect->DeviceAPIDependantEffect());

    _deviceAPIDependantEncapsulator->Device()->DestroyShaderEffect(effect, entity);
}
//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
DeviceAPIDependantTexture2D *DeviceEncapsulator::CreateTexture(Texture2D *texture, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateTexture(texture, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetTexture(ShaderProgramType stage, size_t slot, const Texture *texture) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!texture || texture->Frozen());
    Assert(!texture || texture->Available());

    _deviceAPIDependantEncapsulator->Context()->SetTexture(stage, slot, texture);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyTexture(Texture2D *texture, PDeviceAPIDependantTexture2D& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(entity);
    Assert(&entity == &texture->DeviceAPIDependantTexture2D());

    _deviceAPIDependantEncapsulator->Device()->DestroyTexture(texture, entity);
}
//----------------------------------------------------------------------------
// Render Targets
//----------------------------------------------------------------------------
RenderTarget *DeviceEncapsulator::BackBufferRenderTarget() {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIDependantEncapsulator->Device()->BackBufferRenderTarget();
}
//----------------------------------------------------------------------------
DepthStencil *DeviceEncapsulator::BackBufferDepthStencil() {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIDependantEncapsulator->Device()->BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
const RenderTarget *DeviceEncapsulator::BackBufferRenderTarget() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIDependantEncapsulator->Device()->BackBufferRenderTarget();
}
//----------------------------------------------------------------------------
const DepthStencil *DeviceEncapsulator::BackBufferDepthStencil() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIDependantEncapsulator->Device()->BackBufferDepthStencil();
}
//----------------------------------------------------------------------------
DeviceAPIDependantRenderTarget *DeviceEncapsulator::CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateRenderTarget(renderTarget, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRenderTarget(const RenderTarget *renderTarget, const DepthStencil *depthStencil) {
    Assert(!renderTarget || (renderTarget->Frozen() && renderTarget->Available()));
    Assert(!depthStencil || (depthStencil->Frozen() && depthStencil->Available()));

    _deviceAPIDependantEncapsulator->Context()->SetRenderTarget(renderTarget, depthStencil);
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

    _deviceAPIDependantEncapsulator->Context()->SetRenderTargets(bindings, depthStencil);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyRenderTarget(RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());
    Assert(entity);

    _deviceAPIDependantEncapsulator->Device()->DestroyRenderTarget(renderTarget, entity);
}
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencil *DeviceEncapsulator::CreateDepthStencil(DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());

    return _deviceAPIDependantEncapsulator->Device()->CreateDepthStencil(depthStencil, optionalData);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyDepthStencil(DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());
    Assert(entity);

    _deviceAPIDependantEncapsulator->Device()->DestroyDepthStencil(depthStencil, entity);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Clear(const RenderTarget *renderTarget, const ColorRGBAF& color) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderTarget);
    Assert(renderTarget->Frozen());
    Assert(renderTarget->Available());

    _deviceAPIDependantEncapsulator->Context()->Clear(renderTarget, color);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Clear(const Graphics::DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(depthStencil);
    Assert(depthStencil->Frozen());
    Assert(depthStencil->Available());

    _deviceAPIDependantEncapsulator->Context()->Clear(depthStencil, opts, depth, stencil);
}
//----------------------------------------------------------------------------
// Draw
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawPrimitives(PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIDependantEncapsulator->Context()->DrawPrimitives(primitiveType, startVertex, primitiveCount);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawIndexedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIDependantEncapsulator->Context()->DrawIndexedPrimitives(primitiveType, baseVertex, startIndex, primitiveCount);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawInstancedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIDependantEncapsulator->Context()->DrawInstancedPrimitives(primitiveType, baseVertex, startIndex, primitiveCount, startInstance, instanceCount);
}
//----------------------------------------------------------------------------
// Diagnostics
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
bool DeviceEncapsulator::IsProfilerAttached() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIDependantEncapsulator->Diagnostics()->IsProfilerAttached();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::BeginEvent(const wchar_t *name) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    Assert(IsProfilerAttached());

    return _deviceAPIDependantEncapsulator->Diagnostics()->BeginEvent(name);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::EndEvent() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsProfilerAttached());

    return _deviceAPIDependantEncapsulator->Diagnostics()->EndEvent();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetMarker(const wchar_t *name) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    Assert(IsProfilerAttached());

    return _deviceAPIDependantEncapsulator->Diagnostics()->SetMarker(name);
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *DeviceAPIToCStr(DeviceAPI api) {
    switch (api)
    {
    case Core::Graphics::DeviceAPI::DirectX11:
        return "DirectX11";
    case Core::Graphics::DeviceAPI::OpenGL4:
        return "OpenGL4";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
