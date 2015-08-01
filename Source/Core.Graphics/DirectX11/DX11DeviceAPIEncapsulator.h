#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/AbstractDeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"

#include "Core.Graphics/DirectX11/DX11DeviceWrapper.h"

namespace Core {
namespace Graphics {
class PresentationParameters;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
FWD_REFPTR(DX11ConstantWriter);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11DeviceAPIEncapsulator :
    public  AbstractDeviceAPIEncapsulator
,   private IDeviceAPIEncapsulator
,   private IDeviceAPIContext
,   private IDeviceAPIShaderCompiler
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
,   private IDeviceAPIDiagnostics
#endif
{
public:
    DX11DeviceAPIEncapsulator(DeviceEncapsulator *owner, void *windowHandle, const PresentationParameters& pp);
    virtual ~DX11DeviceAPIEncapsulator();

    const DX11DeviceWrapper& Wrapper() const { return _wrapper; }

    virtual IDeviceAPIEncapsulator *Device() const override { return const_cast<DX11DeviceAPIEncapsulator *>(this); }
    virtual IDeviceAPIContext *Immediate() const override { return const_cast<DX11DeviceAPIEncapsulator *>(this); }
    virtual IDeviceAPIShaderCompiler *ShaderCompiler() const override { return const_cast<DX11DeviceAPIEncapsulator *>(this); }
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *Diagnostics() const override { return const_cast<DX11DeviceAPIEncapsulator *>(this); }
#endif

    virtual void Reset(const PresentationParameters& pp) override;
    virtual void Present() override;
    virtual void ClearState() override;

private:
    DX11DeviceWrapper _wrapper;
    PDX11ConstantWriter _writer;

private:
    virtual const AbstractDeviceAPIEncapsulator *APIEncapsulator() const override { return this; }

    // Diagnostics

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *DeviceDiagnostics() const override { return Diagnostics(); }
#endif

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

    virtual const DeviceAPIDependantConstantWriter *ConstantWriter() const override;

    virtual DeviceAPIDependantResourceBuffer *CreateConstantBuffer(ConstantBuffer *constantBuffer, DeviceResourceBuffer *resourceBuffer) override;
    virtual void DestroyConstantBuffer(ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity) override;

    virtual DeviceAPIDependantShaderEffect *CreateShaderEffect(ShaderEffect *effect) override;
    virtual void DestroyShaderEffect(ShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) override;

    // Textures

    virtual DeviceAPIDependantTexture2D *CreateTexture2D(Texture2D *texture, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyTexture2D(Texture2D *texture, PDeviceAPIDependantTexture2D& entity) override;

    virtual DeviceAPIDependantTextureCube *CreateTextureCube(TextureCube *texture, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyTextureCube(TextureCube *texture, PDeviceAPIDependantTextureCube& entity) override;

    // Render target

    virtual DeviceAPIDependantRenderTarget *CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyRenderTarget(RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) override;

    virtual DeviceAPIDependantDepthStencil *CreateDepthStencil(DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyDepthStencil(DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) override;

    // Render target
    
    virtual RenderTarget *BackBufferRenderTarget() override;
    virtual DepthStencil *BackBufferDepthStencil() override;

    virtual const RenderTarget *BackBufferRenderTarget() const override;
    virtual const DepthStencil *BackBufferDepthStencil() const override;

    virtual void SetRenderTarget(const RenderTarget *renderTarget, const DepthStencil *depthStencil) override;
    virtual void SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const DepthStencil *depthStencil) override;

    virtual void Clear(const RenderTarget *renderTarget, const ColorRGBAF& color) override;
    virtual void Clear(const DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) override;

private: // IDeviceAPIContext

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

private: // IDeviceAPIShaderCompiler impl

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

    //virtual const AbstractDeviceAPIEncapsulator *Encapsulator() const override { return _deviceAPIDependantEncapsulator.get(); }

    virtual bool IsProfilerAttached() const override;

    virtual void BeginEvent(const wchar_t *name) override;
    virtual void EndEvent() override;

    virtual void SetMarker(const wchar_t *name) override;
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const DX11DeviceWrapper *DX11GetDeviceWrapper(const IDeviceAPIEncapsulator *device);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
