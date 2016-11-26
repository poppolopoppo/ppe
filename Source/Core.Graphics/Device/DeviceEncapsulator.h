#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceRevision.h"
#include "Core.Graphics/Device/DeviceStatus.h"

#include "Core/Memory/MemoryTracking.h"
#include "Core/Meta/Event.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
namespace Graphics {
class FAbstractDeviceAPIEncapsulator;
class FDeviceAPIDependantEntity;
class FDeviceSharedEntityPool;
class FPresentationParameters;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceEncapsulator
:   private Meta::FThreadResource
,   private IDeviceAPIContext
,   private IDeviceAPIEncapsulator
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
,   private IDeviceAPIDiagnostics
#endif
{
public:
    using Meta::FThreadResource::CheckThreadId;
    using Meta::FThreadResource::OwnedByThisThread;

    typedef TDelegate<void (*)(FDeviceEncapsulator* )> event_t;

    FDeviceEncapsulator();
    virtual ~FDeviceEncapsulator();

    EDeviceAPI API() const;
    EDeviceStatus Status() const { return _status; }
    DeviceRevision Revision() const { return _revision; }

    const FPresentationParameters& Parameters() const;

    const FMemoryTrackingData& VideoMemory() const { return _videoMemory; }

    IDeviceAPIEncapsulator *Device() const;
    IDeviceAPIContext *Immediate() const;

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    IDeviceAPIDiagnostics *Diagnostics() const;
#endif

    void Create(EDeviceAPI api, void *windowHandle, const FPresentationParameters& presentationParameters);
    void Destroy();

    void Reset(const FPresentationParameters& pp);
    void Present();
    void ClearState();

    TPublicEvent<event_t> OnDeviceCreate()   { return _onDeviceCreate; }
    TPublicEvent<event_t> OnDeviceDestroy()  { return _onDeviceDestroy; }
    TPublicEvent<event_t> OnDeviceReset()    { return _onDeviceReset; }
    TPublicEvent<event_t> OnDevicePresent()  { return _onDevicePresent; }

private:
    TUniquePtr< FAbstractDeviceAPIEncapsulator > _deviceAPIEncapsulator;
    TUniquePtr< FDeviceSharedEntityPool > _deviceSharedEntityPool;

    EDeviceStatus _status;
    DeviceRevision _revision;

    FMemoryTrackingData _videoMemory;

    TEvent<event_t> _onDeviceCreate;
    TEvent<event_t> _onDeviceDestroy;
    TEvent<event_t> _onDeviceReset;
    TEvent<event_t> _onDevicePresent;

private:
    virtual const FAbstractDeviceAPIEncapsulator *APIEncapsulator() const override final;

private: // IDeviceAPIEncapsulator impl

    // Viewport

    virtual void SetViewport(const ViewportF& viewport) override final;
    virtual void SetViewports(const TMemoryView<const ViewportF>& viewports) override final;

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

    virtual FDeviceAPIDependantResourceBuffer *CreateIndexBuffer(IndexBuffer *indexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyIndexBuffer(IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) override final;

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

    virtual FRenderTarget *BackBufferRenderTarget() override final;
    virtual FDepthStencil *BackBufferDepthStencil() override final;

    virtual const FRenderTarget *BackBufferRenderTarget() const override final;
    virtual const FDepthStencil *BackBufferDepthStencil() const override final;

    virtual void SetRenderTarget(const FRenderTarget *renderTarget, const FDepthStencil *depthStencil) override final;
    virtual void SetRenderTargets(const TMemoryView<const FRenderTargetBinding>& bindings, const FDepthStencil *depthStencil) override final;

    virtual void Clear(const FRenderTarget *renderTarget, const ColorRGBAF& color) override final;
    virtual void Clear(const FDepthStencil *depthStencil, EClearOptions opts, float depth, u8 stencil) override final;

    virtual FDeviceAPIDependantRenderTarget *CreateRenderTarget(FRenderTarget *renderTarget, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyRenderTarget(FRenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) override final;

    virtual FDeviceAPIDependantDepthStencil *CreateDepthStencil(FDepthStencil *depthStencil, const TMemoryView<const u8>& optionalData) override final;
    virtual void DestroyDepthStencil(FDepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) override final;

    // diagnostics

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *DeviceDiagnostics() const override final { return Diagnostics(); }
#endif

private: // IDeviceAPIContext

    // Alpha/Raster/Depth State

    virtual void SetBlendState(const FBlendState *state) override final;
    virtual void SetRasterizerState(const FRasterizerState *state) override final;
    virtual void SetDepthStencilState(const FDepthStencilState *state) override final;

    // Index/Vertex Buffer

    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer) override final;
    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) override final;

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

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
private: // IDeviceAPIDiagnosticsEncapsulator() {}

    virtual bool IsProfilerAttached() const override final;

    virtual void BeginEvent(const wchar_t *name) override final;
    virtual void EndEvent() override final;

    virtual void SetMarker(const wchar_t *name) override final;

#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
