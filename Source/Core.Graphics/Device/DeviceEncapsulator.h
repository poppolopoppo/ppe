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
    virtual const FAbstractDeviceAPIEncapsulator *APIEncapsulator() const override;

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

    virtual FRenderTarget *BackBufferRenderTarget() override;
    virtual FDepthStencil *BackBufferDepthStencil() override;

    virtual const FRenderTarget *BackBufferRenderTarget() const override;
    virtual const FDepthStencil *BackBufferDepthStencil() const override;

    virtual void SetRenderTarget(const FRenderTarget *renderTarget, const FDepthStencil *depthStencil) override;
    virtual void SetRenderTargets(const TMemoryView<const FRenderTargetBinding>& bindings, const FDepthStencil *depthStencil) override;

    virtual void Clear(const FRenderTarget *renderTarget, const ColorRGBAF& color) override;
    virtual void Clear(const FDepthStencil *depthStencil, EClearOptions opts, float depth, u8 stencil) override;

    virtual FDeviceAPIDependantRenderTarget *CreateRenderTarget(FRenderTarget *renderTarget, const TMemoryView<const u8>& optionalData) override;
    virtual void DestroyRenderTarget(FRenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) override;

    virtual FDeviceAPIDependantDepthStencil *CreateDepthStencil(FDepthStencil *depthStencil, const TMemoryView<const u8>& optionalData) override;
    virtual void DestroyDepthStencil(FDepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) override;

    // diagnostics

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *DeviceDiagnostics() const override { return Diagnostics(); }
#endif

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
