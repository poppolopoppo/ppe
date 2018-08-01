#pragma once

#include "DirectX11/DX11Includes.h"

#include "Device/AbstractDeviceAPIEncapsulator.h"
#include "Device/DeviceEncapsulator.h"

#include "DirectX11/DX11DeviceWrapper.h"

namespace PPE {
namespace Graphics {
class FPresentationParameters;
} //!namespace Graphics
} //!namespace PPE

namespace PPE {
namespace Graphics {
FWD_REFPTR(DX11ConstantWriter);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11DeviceAPIEncapsulator :
    public  FAbstractDeviceAPIEncapsulator
,   private IDeviceAPIEncapsulator
,   private IDeviceAPIContext
#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
,   private IDeviceAPIDiagnostics
#endif
{
public:
    FDX11DeviceAPIEncapsulator(FDeviceEncapsulator *owner, void *windowHandle, const FPresentationParameters& pp);
    virtual ~FDX11DeviceAPIEncapsulator();

    const FDX11DeviceWrapper& Wrapper() const { return _wrapper; }

    virtual IDeviceAPIEncapsulator *Device() const override final { return remove_const(this); }
    virtual IDeviceAPIContext *Immediate() const override final { return remove_const(this); }

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *Diagnostics() const override final { return remove_const(this); }
#endif

    virtual void Reset(const FPresentationParameters& pp) override final;
    virtual void Present() override final;
    virtual void ClearState() override final;

private:
    FDX11DeviceWrapper _wrapper;
    PDX11ConstantWriter _writer;

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
    u16 _useDebugDrawEvents = 0;
    u16 _framesToCapture = 0;
    u16 _framesCapturing = 0;
#endif

private:
    virtual const FAbstractDeviceAPIEncapsulator *APIEncapsulator() const override final { return this; }

    // Diagnostics

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *DeviceDiagnostics() const override final { return Diagnostics(); }
#endif

private: // IDeviceAPIEncapsulator impl

    // Viewport

    virtual void SetViewport(const FViewport& viewport) override final;
    virtual void SetViewports(const TMemoryView<const FViewport>& viewports) override final;

    // Alpha/Raster/Depth State

    virtual FDeviceAPIDependantBlendState *CreateBlendState(FBlendState *state) override final;
    virtual void DestroyBlendState(FBlendState *state, PDeviceAPIDependantBlendState& entity) override final;

    virtual FDeviceAPIDependantRasterizerState *CreateRasterizerState(FRasterizerState *state) override final;
    virtual void DestroyRasterizerState(FRasterizerState *state, PDeviceAPIDependantRasterizerState& entity) override final;

    virtual FDeviceAPIDependantDepthStencilState *CreateDepthStencilState(FDepthStencilState *state) override final;
    virtual void DestroyDepthStencilState(FDepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) override final;

    virtual FDeviceAPIDependantSamplerState *CreateSamplerState(FSamplerState *state) override final;
    virtual void DestroySamplerState(FSamplerState *state, PDeviceAPIDependantSamplerState& entity) override final;

    // Index/Vertex Buffer

    virtual FDeviceAPIDependantVertexDeclaration *CreateVertexDeclaration(FVertexDeclaration *declaration) override final;
    virtual void DestroyVertexDeclaration(FVertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) override final;

    virtual FDeviceAPIDependantResourceBuffer *CreateIndexBuffer(FIndexBuffer *indexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyIndexBuffer(FIndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) override final;

    virtual FDeviceAPIDependantResourceBuffer *CreateVertexBuffer(FVertexBuffer *vertexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyVertexBuffer(FVertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) override final;

    // Shaders

    virtual const FDeviceAPIDependantConstantWriter *ConstantWriter() const override final;

    virtual FDeviceAPIDependantResourceBuffer *CreateConstantBuffer(FConstantBuffer *constantBuffer, FDeviceResourceBuffer *resourceBuffer) override final;
    virtual void DestroyConstantBuffer(FConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity) override final;

    virtual FDeviceAPIDependantShaderProgram* CreateShaderProgram(FShaderProgram* program) override final;
    virtual void DestroyShaderProgram(FShaderProgram* program, PDeviceAPIDependantShaderProgram& entity) override final;

    virtual FDeviceAPIDependantShaderEffect *CreateShaderEffect(FShaderEffect *effect) override final;
    virtual void DestroyShaderEffect(FShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) override final;

    // Textures

    virtual FDeviceAPIDependantTexture2D *CreateTexture2D(FTexture2D *texture, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyTexture2D(FTexture2D *texture, PDeviceAPIDependantTexture2D& entity) override final;

    virtual FDeviceAPIDependantTextureCube *CreateTextureCube(FTextureCube *texture, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyTextureCube(FTextureCube *texture, PDeviceAPIDependantTextureCube& entity) override final;

    // Render target

    virtual FDeviceAPIDependantRenderTarget *CreateRenderTarget(FRenderTarget *renderTarget, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyRenderTarget(FRenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) override final;

    virtual FDeviceAPIDependantDepthStencil *CreateDepthStencil(FDepthStencil *depthStencil, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyDepthStencil(FDepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) override final;

    // Render target

    virtual FRenderTarget *BackBufferRenderTarget() override final;
    virtual FDepthStencil *BackBufferDepthStencil() override final;

    virtual const FRenderTarget *BackBufferRenderTarget() const override final;
    virtual const FDepthStencil *BackBufferDepthStencil() const override final;

    virtual void SetRenderTarget(const FRenderTarget *renderTarget, const FDepthStencil *depthStencil) override final;
    virtual void SetRenderTargets(const TMemoryView<const FRenderTargetBinding>& bindings, const FDepthStencil *depthStencil) override final;

    virtual void Clear(const FRenderTarget *renderTarget, const FLinearColor& color) override final;
    virtual void Clear(const FDepthStencil *depthStencil, EClearOptions opts, float depth, u8 stencil) override final;

private: // IDeviceAPIContext

    // Alpha/Raster/Depth State

    virtual void SetBlendState(const FBlendState *state) override final;
    virtual void SetRasterizerState(const FRasterizerState *state) override final;
    virtual void SetDepthStencilState(const FDepthStencilState *state) override final;

    // Index/Vertex Buffer

    virtual void SetIndexBuffer(const FIndexBuffer *indexBuffer) override final;
    virtual void SetIndexBuffer(const FIndexBuffer *indexBuffer, size_t offset) override final;

    virtual void SetVertexBuffer(const FVertexBuffer *vertexBuffer) override final;
    virtual void SetVertexBuffer(const FVertexBuffer *vertexBuffer, u32 vertexOffset) override final;
    virtual void SetVertexBuffer(const TMemoryView<const FVertexBufferBinding>& bindings) override final;

    // Shaders

    virtual void SetShaderEffect(const FShaderEffect *effect) override final;

    virtual void SetConstantBuffer(EShaderProgramType stage, size_t slot, const FConstantBuffer *constantBuffer) override final;
    virtual void SetConstantBuffers(EShaderProgramType stage, const TMemoryView<const FConstantBuffer *>& constantBuffers) override final;

    virtual void SetTexture(EShaderProgramType stage, size_t slot, const FTexture *texture) override final;
    virtual void SetTextures(EShaderProgramType stage, const TMemoryView<const FTexture *>& textures) override final;

    virtual void SetSamplerState(EShaderProgramType stage, size_t slot, const FSamplerState *state) override final;
    virtual void SetSamplerStates(EShaderProgramType stage, const TMemoryView<const FSamplerState *>& states) override final;

    // Draw

    virtual void DrawPrimitives(EPrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) override final;
    virtual void DrawIndexedPrimitives(EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) override final;
    virtual void DrawInstancedPrimitives(EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) override final;

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
private: // IDeviceAPIDiagnosticsEncapsulator() {}

    //virtual const FAbstractDeviceAPIEncapsulator *Encapsulator() const override final { return _deviceAPIDependantEncapsulator.get(); }

    virtual bool UseDebugDrawEvents() const override final;
    virtual void ToggleDebugDrawEvents(bool enabled) override final;

    virtual void SetMarker(const FWStringView& name) override final;
    virtual void BeginEvent(const FWStringView& name) override final;
    virtual void EndEvent() override final;

    virtual bool IsProfilerAttached() const override final;
    virtual bool LaunchProfiler() override final;
    virtual bool LaunchProfilerAndTriggerCapture() override final;

    virtual bool IsCapturingFrame() const override final;
    virtual void SetCaptureWindow(void* hwnd) override final;
    virtual void TriggerCapture() override final;
    virtual void TriggerMultiFrameCapture(size_t numFrames) override final;

#endif //!WITH_PPE_GRAPHICS_DIAGNOSTICS
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FDX11DeviceWrapper *DX11GetDeviceWrapper(const IDeviceAPIEncapsulator *device);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
