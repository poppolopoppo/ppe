#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/AbstractDeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"

#include "Core.Graphics/DirectX11/DX11DeviceWrapper.h"

namespace Core {
namespace Graphics {
class FPresentationParameters;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
FWD_REFPTR(DX11ConstantWriter);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11DeviceAPIEncapsulator :
    public  FAbstractDeviceAPIEncapsulator
,   private IDeviceAPIEncapsulator
,   private IDeviceAPIContext
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
,   private IDeviceAPIDiagnostics
#endif
{
public:
    FDX11DeviceAPIEncapsulator(FDeviceEncapsulator *owner, void *windowHandle, const FPresentationParameters& pp);
    virtual ~FDX11DeviceAPIEncapsulator();

    const FDX11DeviceWrapper& Wrapper() const { return _wrapper; }

    virtual IDeviceAPIEncapsulator *Device() const override { return remove_const(this); }
    virtual IDeviceAPIContext *Immediate() const override { return remove_const(this); }

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *Diagnostics() const override { return remove_const(this); }
#endif

    virtual void Reset(const FPresentationParameters& pp) override;
    virtual void Present() override;
    virtual void ClearState() override;

private:
    FDX11DeviceWrapper _wrapper;
    PDX11ConstantWriter _writer;

private:
    virtual const FAbstractDeviceAPIEncapsulator *APIEncapsulator() const override { return this; }

    // Diagnostics

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *DeviceDiagnostics() const override { return Diagnostics(); }
#endif

private: // IDeviceAPIEncapsulator impl

    // Viewport

    virtual void SetViewport(const ViewportF& viewport) override;
    virtual void SetViewports(const TMemoryView<const ViewportF>& viewports) override;

    // Alpha/Raster/Depth State

    virtual FDeviceAPIDependantBlendState *CreateBlendState(FBlendState *state) override;
    virtual void DestroyBlendState(FBlendState *state, PDeviceAPIDependantBlendState& entity) override;

    virtual FDeviceAPIDependantRasterizerState *CreateRasterizerState(FRasterizerState *state) override;
    virtual void DestroyRasterizerState(FRasterizerState *state, PDeviceAPIDependantRasterizerState& entity) override;

    virtual FDeviceAPIDependantDepthStencilState *CreateDepthStencilState(FDepthStencilState *state) override;
    virtual void DestroyDepthStencilState(FDepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) override;

    virtual FDeviceAPIDependantSamplerState *CreateSamplerState(FSamplerState *state) override;
    virtual void DestroySamplerState(FSamplerState *state, PDeviceAPIDependantSamplerState& entity) override;

    // Index/Vertex Buffer

    virtual FDeviceAPIDependantVertexDeclaration *CreateVertexDeclaration(FVertexDeclaration *declaration) override;
    virtual void DestroyVertexDeclaration(FVertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) override;

    virtual FDeviceAPIDependantResourceBuffer *CreateIndexBuffer(IndexBuffer *indexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) override;
    virtual void DestroyIndexBuffer(IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) override;

    virtual FDeviceAPIDependantResourceBuffer *CreateVertexBuffer(FVertexBuffer *vertexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) override;
    virtual void DestroyVertexBuffer(FVertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) override;

    // Shaders

    virtual const FDeviceAPIDependantConstantWriter *ConstantWriter() const override;

    virtual FDeviceAPIDependantResourceBuffer *CreateConstantBuffer(FConstantBuffer *constantBuffer, FDeviceResourceBuffer *resourceBuffer) override;
    virtual void DestroyConstantBuffer(FConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity) override;

    virtual FDeviceAPIDependantShaderProgram* CreateShaderProgram(FShaderProgram* program) override;
    virtual void DestroyShaderProgram(FShaderProgram* program, PDeviceAPIDependantShaderProgram& entity) override;

    virtual FDeviceAPIDependantShaderEffect *CreateShaderEffect(FShaderEffect *effect) override;
    virtual void DestroyShaderEffect(FShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) override;

    // Textures

    virtual FDeviceAPIDependantTexture2D *CreateTexture2D(FTexture2D *texture, const TMemoryView<const u8>& optionalData) override;
    virtual void DestroyTexture2D(FTexture2D *texture, PDeviceAPIDependantTexture2D& entity) override;

    virtual FDeviceAPIDependantTextureCube *CreateTextureCube(FTextureCube *texture, const TMemoryView<const u8>& optionalData) override;
    virtual void DestroyTextureCube(FTextureCube *texture, PDeviceAPIDependantTextureCube& entity) override;

    // Render target

    virtual FDeviceAPIDependantRenderTarget *CreateRenderTarget(FRenderTarget *renderTarget, const TMemoryView<const u8>& optionalData) override;
    virtual void DestroyRenderTarget(FRenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) override;

    virtual FDeviceAPIDependantDepthStencil *CreateDepthStencil(FDepthStencil *depthStencil, const TMemoryView<const u8>& optionalData) override;
    virtual void DestroyDepthStencil(FDepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) override;

    // Render target

    virtual FRenderTarget *BackBufferRenderTarget() override;
    virtual FDepthStencil *BackBufferDepthStencil() override;

    virtual const FRenderTarget *BackBufferRenderTarget() const override;
    virtual const FDepthStencil *BackBufferDepthStencil() const override;

    virtual void SetRenderTarget(const FRenderTarget *renderTarget, const FDepthStencil *depthStencil) override;
    virtual void SetRenderTargets(const TMemoryView<const FRenderTargetBinding>& bindings, const FDepthStencil *depthStencil) override;

    virtual void Clear(const FRenderTarget *renderTarget, const ColorRGBAF& color) override;
    virtual void Clear(const FDepthStencil *depthStencil, EClearOptions opts, float depth, u8 stencil) override;

private: // IDeviceAPIContext

    // Alpha/Raster/Depth State

    virtual void SetBlendState(const FBlendState *state) override;
    virtual void SetRasterizerState(const FRasterizerState *state) override;
    virtual void SetDepthStencilState(const FDepthStencilState *state) override;

    // Index/Vertex Buffer

    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer) override;
    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) override;

    virtual void SetVertexBuffer(const FVertexBuffer *vertexBuffer) override;
    virtual void SetVertexBuffer(const FVertexBuffer *vertexBuffer, u32 vertexOffset) override;
    virtual void SetVertexBuffer(const TMemoryView<const FVertexBufferBinding>& bindings) override;

    // Shaders

    virtual void SetShaderEffect(const FShaderEffect *effect) override;

    virtual void SetConstantBuffer(EShaderProgramType stage, size_t slot, const FConstantBuffer *constantBuffer) override;
    virtual void SetConstantBuffers(EShaderProgramType stage, const TMemoryView<const FConstantBuffer *>& constantBuffers) override;

    virtual void SetTexture(EShaderProgramType stage, size_t slot, const FTexture *texture) override;
    virtual void SetTextures(EShaderProgramType stage, const TMemoryView<const FTexture *>& textures) override;

    virtual void SetSamplerState(EShaderProgramType stage, size_t slot, const FSamplerState *state) override;
    virtual void SetSamplerStates(EShaderProgramType stage, const TMemoryView<const FSamplerState *>& states) override;

    // Draw

    virtual void DrawPrimitives(EPrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) override;
    virtual void DrawIndexedPrimitives(EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) override;
    virtual void DrawInstancedPrimitives(EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) override;

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
private: // IDeviceAPIDiagnosticsEncapsulator() {}

    //virtual const FAbstractDeviceAPIEncapsulator *Encapsulator() const override { return _deviceAPIDependantEncapsulator.get(); }

    virtual bool IsProfilerAttached() const override;

    virtual void BeginEvent(const wchar_t *name) override;
    virtual void EndEvent() override;

    virtual void SetMarker(const wchar_t *name) override;
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FDX11DeviceWrapper *DX11GetDeviceWrapper(const IDeviceAPIEncapsulator *device);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
