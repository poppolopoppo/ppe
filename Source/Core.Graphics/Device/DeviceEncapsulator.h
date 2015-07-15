#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI.h"

#include "Core/Meta/ThreadResource.h"

namespace Core {
namespace Graphics {
class AbstractDeviceAPIEncapsulator;
class PresentationParameters;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class DeviceStatus {
    Invalid     = 0,
    Normal      = 1,
    Reset       = 3,
    Lost        = 4,
    Destroy     = 5,
};
//----------------------------------------------------------------------------
const char *DeviceStatusToCStr(DeviceStatus status);
//----------------------------------------------------------------------------
CORE_STRONGLYTYPED_NUMERIC_DEF(u64, DeviceRevision);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceEncapsulator 
:   private Meta::ThreadResource 
,   private IDeviceAPIContext
,   private IDeviceAPIEncapsulator
,   private IDeviceAPIShaderCompiler
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
,   private IDeviceAPIDiagnostics
#endif
{
public:
    using Meta::ThreadResource::CheckThreadId;
    using Meta::ThreadResource::OwnedByThisThread;

    DeviceEncapsulator();
    virtual ~DeviceEncapsulator();

    DeviceAPI API() const;
    DeviceStatus Status() const { return _status; }
    DeviceRevision Revision() const { return _revision; }

    const PresentationParameters& Parameters() const;

    IDeviceAPIEncapsulator *Device() const;
    IDeviceAPIContext *Immediate() const;
    IDeviceAPIShaderCompiler *ShaderCompiler() const;

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    IDeviceAPIDiagnostics *Diagnostics() const;
#endif

    void Create(DeviceAPI api, void *windowHandle, const PresentationParameters& presentationParameters);
    void Destroy();

    void Reset(const PresentationParameters& pp);
    void Present();
    void ClearState();

private:
    UniquePtr< AbstractDeviceAPIEncapsulator > _deviceAPIEncapsulator;
    DeviceStatus _status;
    DeviceRevision _revision;

private:
    virtual const AbstractDeviceAPIEncapsulator *APIEncapsulator() const override;

private: // IDeviceAPIEncapsulator impl

    // Viewport
    
    virtual void SetViewport(const ViewportF& viewport) override;
    virtual void SetViewports(const MemoryView<const ViewportF>& viewports) override;

    // Alpha/Raster/Depth State

    virtual DeviceAPIDependantBlendState *CreateBlendState(BlendState *state) override;
    virtual void DestroyBlendState(BlendState *state, PDeviceAPIDependantBlendState& entity) override;

    virtual DeviceAPIDependantRasterizerState *CreateRasterizerState(RasterizerState *state) override;
    virtual void DestroyRasterizerState(RasterizerState *state, PDeviceAPIDependantRasterizerState& entity) override;

    virtual DeviceAPIDependantDepthStencilState *CreateDepthStencilState(DepthStencilState *state) override;
    virtual void DestroyDepthStencilState(DepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) override;

    virtual DeviceAPIDependantSamplerState *CreateSamplerState(SamplerState *state) override;
    virtual void DestroySamplerState(SamplerState *state, PDeviceAPIDependantSamplerState& entity) override;

    // Index/Vertex Buffer

    virtual DeviceAPIDependantVertexDeclaration *CreateVertexDeclaration(VertexDeclaration *declaration) override;
    virtual void DestroyVertexDeclaration(VertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) override;

    virtual DeviceAPIDependantResourceBuffer *CreateIndexBuffer(IndexBuffer *indexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyIndexBuffer(IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) override;

    virtual DeviceAPIDependantResourceBuffer *CreateVertexBuffer(VertexBuffer *vertexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyVertexBuffer(VertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) override;

    // Shaders

    virtual DeviceAPIDependantResourceBuffer *CreateConstantBuffer(ConstantBuffer *constantBuffer, DeviceResourceBuffer *resourceBuffer, PDeviceAPIDependantConstantWriter& writer) override;
    virtual void DestroyConstantBuffer(ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity, PDeviceAPIDependantConstantWriter& writer) override;

    virtual DeviceAPIDependantShaderEffect *CreateShaderEffect(ShaderEffect *effect) override;
    virtual void DestroyShaderEffect(ShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) override;

    // Textures

    virtual DeviceAPIDependantTexture2D *CreateTexture2D(Texture2D *texture, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyTexture2D(Texture2D *texture, PDeviceAPIDependantTexture2D& entity) override;

    virtual DeviceAPIDependantTextureCube *CreateTextureCube(TextureCube *texture, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyTextureCube(TextureCube *texture, PDeviceAPIDependantTextureCube& entity) override;

    // Render target
    
    virtual RenderTarget *BackBufferRenderTarget() override;
    virtual DepthStencil *BackBufferDepthStencil() override;

    virtual const RenderTarget *BackBufferRenderTarget() const override;
    virtual const DepthStencil *BackBufferDepthStencil() const override;

    virtual void SetRenderTarget(const RenderTarget *renderTarget, const DepthStencil *depthStencil) override;
    virtual void SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const DepthStencil *depthStencil) override;

    virtual void Clear(const RenderTarget *renderTarget, const ColorRGBAF& color) override;
    virtual void Clear(const DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) override;

    virtual DeviceAPIDependantRenderTarget *CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyRenderTarget(RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) override;

    virtual DeviceAPIDependantDepthStencil *CreateDepthStencil(DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyDepthStencil(DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) override;

private: // IDeviceAPIContextEncapsulator

    // Alpha/Raster/Depth State

    virtual void SetBlendState(const BlendState *state) override;
    virtual void SetRasterizerState(const RasterizerState *state) override;
    virtual void SetDepthStencilState(const DepthStencilState *state) override;

    // Index/Vertex Buffer

    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer) override;
    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) override;

    virtual void SetVertexBuffer(const VertexBuffer *vertexBuffer) override;
    virtual void SetVertexBuffer(const VertexBuffer *vertexBuffer, u32 vertexOffset) override;
    virtual void SetVertexBuffer(const MemoryView<const VertexBufferBinding>& bindings) override;

    // Shaders

    virtual void SetShaderEffect(const ShaderEffect *effect) override;

    virtual void SetConstantBuffer(ShaderProgramType stage, size_t slot, const ConstantBuffer *constantBuffer) override;
    virtual void SetConstantBuffers(ShaderProgramType stage, const MemoryView<const ConstantBuffer *>& constantBuffers) override;

    virtual void SetTexture(ShaderProgramType stage, size_t slot, const Texture *texture) override;
    virtual void SetTextures(ShaderProgramType stage, const MemoryView<const Texture *>& textures) override;

    virtual void SetSamplerState(ShaderProgramType stage, size_t slot, const SamplerState *state) override;
    virtual void SetSamplerStates(ShaderProgramType stage, const MemoryView<const SamplerState *>& states) override;

    // Draw

    virtual void DrawPrimitives(PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) override;
    virtual void DrawIndexedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) override;
    virtual void DrawInstancedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) override;

private: // IDeviceAPIShaderCompiler

    virtual DeviceAPIDependantShaderProgram *CreateShaderProgram(
        ShaderProgram *program,
        const char *entryPoint,
        ShaderCompilerFlags flags,
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration) override;
    virtual void DestroyShaderProgram(ShaderProgram *program, PDeviceAPIDependantShaderProgram& entity) override;

    virtual void PreprocessShaderProgram(
        RAWSTORAGE(Shader, char)& output,
        const ShaderProgram *program,
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration) override;

    virtual void ReflectShaderProgram(
        ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
        VECTOR(Shader, ShaderProgramTexture)& textures,
        const ShaderProgram *program) override;

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
private: // IDeviceAPIDiagnosticsEncapsulator() {}

    virtual bool IsProfilerAttached() const override;

    virtual void BeginEvent(const wchar_t *name) override;
    virtual void EndEvent() override;

    virtual void SetMarker(const wchar_t *name) override;

#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
